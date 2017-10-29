
/**
 * Como argumentos recibe el número de puerto en el que debe realizar la
 * conexión
 *
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <string>
#include <chrono>
#include <cstring>
#include <limits.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include <sstream>
#include <netdb.h>
#include <iomanip>
#include <errno.h>
#include <sys/stat.h>

using namespace std;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;

const char* CONNECTION = "Connection sucessfull";
const char* RECEIVED = "Message received";
const string SERVER = "server";
const char* STOPPING = "Stopping server";
const char* STARTING = "Successfully started server";
const char* LOCK = "server.lock";
const char* LOG = "server.log";
const int CHRONO = 30;
const int NUM_ARGS = 2;
bool running = true;


static bool checkChrono(auto& startTime) {
	auto currentTime = chrono::high_resolution_clock::now();
	if (chrono::duration_cast<chrono::seconds>(currentTime - startTime).count() >= CHRONO) {
		// Enviar informacion del log
		startTime = chrono::high_resolution_clock::now();
		return true;
	} else
		return false;
}

static void handleSignal(int sign) {
	if (sign == SIGINT || sign == SIGSTOP) {
		syslog(LOG_NOTICE, "Stopping server");
		closelog();
		exit(EXIT_SUCCESS);
		running = false;
		signal(SIGINT, SIG_DFL);
	} else if (sign == SIGCHLD) {
		cerr << endl;
	}
}

static void daemonize() {
#ifdef DEBUG
	cout << "Starting to daemonize" << endl;
#endif
	pid_t pid, sid;
	int lock;
	pid = fork();
	if (pid < 0) {
		exit(1);
	}
	if (pid > 0) {
		exit(0);
	}
	umask(0);
	sid = setsid();
	if (sid < 0) {
		cerr << "Error: SID < 0";
		exit(1);
	}
	if (chdir("/") <  0) {
		cerr << "Error trying to move to /";
		exit(1);
	}
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	// Bloqueamos que se puedan instanciar más demonios
	lock = open(LOCK, O_RDWR | O_CREAT, 0640);
	if (lock < 0) {
		exit(1); // No se instancia uno nuevo
	}
	if (lockf(lock, F_TLOCK, 0) < 0) {
		exit(0); // No se puede bloquear
	}
	// Fichero de log
	openlog(LOG, LOG_PID, LOG_DAEMON);
	syslog(LOG_NOTICE, "Successfully started server");
	signal(SIGCHLD, SIG_IGN);
	signal(SIGINT, handleSignal);
	signal(SIGSTOP, handleSignal);
}

static void createConnection(const int port, int& sockfd, int& newSockFd, socklen_t& clientLength, struct sockaddr_in& servAddr, struct sockaddr_in& clientAddr) {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cerr << "Error creating socket file descriptor" << endl;
		exit(-1);
	}
	bzero((char*)& servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr*)& servAddr, sizeof(servAddr)) < 0) {
		cerr << "Error trying to bind" << endl;
		exit(-1);
	}
	listen(sockfd, 5);
	clientLength = sizeof(clientAddr);
}

static void doWork(int& sockfd, int& newSockFd, socklen_t& clientLength, struct sockaddr_in& servAddr, struct sockaddr_in& clientAddr) {
	auto startTime = chrono::high_resolution_clock::now();
	char buffer[256];
	int i;
	while (true) {
		newSockFd = accept(sockfd, (struct sockaddr*)& clientAddr, &clientLength);
		if (newSockFd < 0) {
			cerr << "Error creating socket file descriptor for client" << endl;
			exit(-1);
		}
		pid_t pid = fork();
		if (pid < 0) {
			cerr << "Error trying to create a new process" << endl;
			exit(-1);
		}
		if (pid == 0) {
			close(sockfd);
			bzero(buffer, 256);
			string pidStr;
			i = read(newSockFd, buffer, sizeof(long));
			cout << "Nuevo cliente" << endl;
			if (i < 0) {
				cerr << "Error trying to read pid from client" << endl;
			} else {
				pidStr = buffer;
			}
			bzero(buffer, 256);
			send(newSockFd, CONNECTION, strlen(CONNECTION), 0); // Avisamos que la conexion ha sido realizada
			while (running) {
				i = read(newSockFd, buffer, 255);
				if (i > 0) {
					string str(buffer);
					str += "(" + pidStr + ")";
					syslog(LOG_NOTICE, str.c_str());
				}
				bzero(buffer, 256); // Limpiamos el buffer
				send(newSockFd, RECEIVED, strlen(RECEIVED), 0);
			}
			exit(0);
		} else {
			close(newSockFd);
		}
	}
	close(sockfd);
}

int main(int argc, char const* argv[]) {
	if (argc != NUM_ARGS) {
		cerr << "Error in arguments" << endl;
		cerr << "Usage: ./server <port>" << endl;
		exit(EXIT_SUCCESS);
	}
	int sockfd, newSockFd;
	socklen_t clientLength;
	struct sockaddr_in servAddr, clientAddr;
	// Convertimos al servidor en un demonio
	daemonize();
	createConnection(atoi(argv[1]), sockfd, newSockFd, clientLength, servAddr, clientAddr);
	doWork(sockfd, newSockFd, clientLength, servAddr, clientAddr);
	return 0;
}