


/**
 * Como argumentos recibe el nombre del servidor (localhost en nuestro caso)
 * y el número de puerto en el que debe realizar la conexión
 *
 **/

#include "Client.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <netdb.h>
#include <string>
#include <iomanip>
#include <sstream>
#include <signal.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <cstring>

using namespace std;

const int Client::BUFFER_LENGTH = (10 * (sizeof(struct inotify_event) + NAME_MAX + 1));
const string Client::CLIENT = "Client";
const char* STOPPING = "Stopping daemon";
const char* STARTING = "Successfully started Client";
const char* LOCK = "Client.lock";
const char* LOG = "Client.log";
static bool running = true;
const int DELAY = 1;

using namespace std;

Client::Client() : inotifyFileDescriptor(0),
	eventWatch(0) {}

Client::~Client() { }

Client::Client(char const* host, int port, const int arguments, vector<const char*> pathname) : inotifyFileDescriptor(0),
	eventWatch(0) {
	portNumber = port;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cerr << "Error creating socket file descriptor" << endl;
		exit(-1);
	}
	server = gethostbyname(host); // Dirección del servidor
	if (server == NULL) {
		cerr << "Error trying to get hostbyname for server" << endl;
		exit(-1);
	}
	bzero((char*)& servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)& servAddr.sin_addr.s_addr, server->h_length);
	servAddr.sin_port = htons(portNumber);
	if (connect(sockfd, (struct sockaddr*)& servAddr, sizeof(servAddr)) < 0) {
		cerr << "Errory trying to connect to server" << endl;
		exit(-1);
	}
	start(arguments, pathname);
}



const char* Client::showEvent(struct inotify_event* event) {
	string resultString = "";
	resultString += "\tWatch: " + event->wd;
	resultString += "\tCookie: " + event->cookie;
	if (event->mask & IN_CREATE) {
		resultString += "\tmask: IN_CREATE ";
	}
	if (event->mask & IN_MODIFY) {
		resultString += "\tmask: IN_MODIFY ";
	}
	if (event->mask & IN_ACCESS) {
		resultString += "\tmask: IN_ACCESS ";
	}
	if (event->mask & IN_ATTRIB) {
		resultString += "\tmask: IN_ATTRIB ";
	}
	if (event->mask & IN_CLOSE_WRITE) {
		resultString += "\tmask: IN_CLOSE_WRITE ";
	}
	if (event->mask & IN_CLOSE_NOWRITE) {
		resultString += "\tmask: IN_CLOSE_NOWRITE ";
	}
	if (event->mask & IN_DELETE) {
		resultString += "\tmask: IN_DELETE ";
	}
	if (event->mask & IN_DELETE_SELF) {
		resultString += "\tmask: IN_DELETE_SELF ";
	}
	if (event->mask & IN_MOVE_SELF) {
		resultString += "\tmask: IN_MOVE_SELF ";
	}
	if (event->mask & IN_MOVED_FROM) {
		resultString += "\tmask: IN_MOVED_FROM ";
	}
	if (event->mask & IN_MOVED_TO) {
		resultString += "\tmask: IN_MOVED_TO ";
	}
	if (event->mask & IN_OPEN) {
		resultString += "\tmask: IN_OPEN ";
	}
	if (event->mask & IN_ALL_EVENTS) {
		resultString += "\tmask: IN_ALL_EVENTS ";
	}
	if (event->mask & IN_MOVE) {
		resultString += "\tmask: IN_MOVE ";
	}
	if (event->mask & IN_CLOSE) {
		resultString += "\tmask: IN_CLOSE ";
	}
	if (event->mask & IN_DONT_FOLLOW) {
		resultString += "\tmask: IN_DONT_FOLLOW ";
	}
	if (event->mask & IN_MASK_ADD) {
		resultString += "\tmask: IN_MASK_ADD ";
	}
	if (event->mask & IN_ONESHOT) {
		resultString += "\tmask: IN_ONESHOT ";
	}
	if (event->mask & IN_ONLYDIR) {
		resultString += "\tmask: IN_ONLYDIR ";
	}
	if (event->mask & IN_IGNORED) {
		resultString += "\tmask: IN_IGNORED ";
	}
	if (event->mask & IN_ISDIR) {
		resultString += "\tmask: IN_ISDIR ";
	}
	if (event->mask & IN_Q_OVERFLOW) {
		resultString += "\tmask: IN_Q_OVERFLOW ";
	}
	if (event->mask & IN_UNMOUNT) {
		resultString += "\tmask: IN_UNMOUNT ";
	}
	resultString += "\n";
	if (event->len > 0) {
		resultString += "\tName: ";
		resultString += event->name;
	}
	return resultString.c_str();
}


/**
 * Método encargado de inicializar el demonio
 * Creamos el demonio (PID)
 * Matamos al padre
 * Asignamos la nueva máscara
 * Lo hacemos lider de sesión (SID)
 **/
void Client::start(const int arguments, vector<const char*> pathname) {
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
	long pid = (long) getpid();
	ostringstream ss;
	ss << pid;
	string pidStr = ss.str();
	write(lock, pidStr.c_str(), strlen(pidStr.c_str()));
	// Fichero de log
	openlog(LOG, LOG_PID, LOG_DAEMON);
	syslog(LOG_NOTICE, "Successfully started Client");
	signal(SIGCHLD, SIG_IGN);
	signal(SIGINT, handleSignal);
	signal(SIGSTOP, handleSignal);
	run(arguments, pathname);
}


void Client::handleSignal(int sign) {
	if (sign == SIGINT || sign == SIGSTOP) {
		syslog(LOG_NOTICE, "Stopping daemon");
		closelog();
		exit(EXIT_SUCCESS);
		running = false;
		signal(SIGINT, SIG_DFL);
	} else if (sign == SIGCHLD) {
		cerr << "NR" << endl;
	}
}


/**
 * Método principal de trabajo del demonio
 **/
void Client::run(const int arguments, vector<const char*> pathname) {
	// Enviamos el pid para identificar al cliente
	long pid = (long) getpid();
	ostringstream ss;
	ss << pid;
	string pidStr = ss.str();
	send(sockfd, pidStr.c_str(), sizeof(pidStr.c_str()), 0);
	while (running) {
		ssize_t numRead;
		char* buffer;
		char readBuffer[BUFFER_LENGTH];
		inotifyFileDescriptor = inotify_init();
		if (inotifyFileDescriptor == -1) {
			cerr << "Error: inotifyFileDescriptor = -1" << endl;
			exit(1);
		}
		for (int i = 0; i < arguments; ++i) {
			eventWatch = inotify_add_watch(inotifyFileDescriptor, pathname[i], IN_ALL_EVENTS);
			if (eventWatch == -1) {
				cerr << "Error: eventWatch = -1" << endl;
				exit(1);
			}
		}
		for ( ; ; ) {
			numRead = read(inotifyFileDescriptor, readBuffer, BUFFER_LENGTH);
			for (buffer = readBuffer; buffer < readBuffer + numRead; ) {
				event = (struct inotify_event*) buffer;
				const char* strEvent = showEvent(event);
				i = write(sockfd, strEvent, strlen(strEvent));
				if (i < 0) {
					cerr << "Couldn't be able to write in the socket" << endl;
				}
				buffer += sizeof(struct inotify_event*) + event->len;
			}
			i = read(sockfd, buffer, 255); // Leemos la confirmacion
			syslog(LOG_NOTICE, buffer);
			bzero(buffer, 256);
		}
		sleep(DELAY);
	}
	// Si salimos del bucle se acaba la ejecucion
	syslog(LOG_NOTICE, "Stopping daemon");
	closelog();
	close(sockfd);
	exit(EXIT_SUCCESS);
}