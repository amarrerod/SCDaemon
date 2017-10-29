#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <chrono>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <iomanip>
#include <errno.h>
#include <vector>
#include <dirent.h>


using namespace std;

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
const string DAEMON = "DAEMON";
const char* STOPPING = "Stopping daemon";
const char* STARTING = "Successfully started DAEMON";
const char* LOCK = "DAEMON.lock";
const char* LOG = "DAEMON.log";
const char* CONNECTION = "Connection sucessfull";
const char* IS_IN_DIR = "IS_IN_DIR";
static bool running = true;
const int DELAY = 1;
const int NUM_ARGS = 4;

#define DEBUG

static const char* showEvent(struct inotify_event* event, bool& inDir) {
	string resultString = "";
	if (event->len > 0) {
		resultString += "Name: ";
		resultString += event->name;
		resultString +=  " ";
	}
	//resultString += " Watch: " + event->wd;
	//resultString += " Cookie: " + event->cookie;
	if (event->mask & IN_CREATE) {
		resultString += "CREATE ";
	}
	if (event->mask & IN_MODIFY) {
		resultString += "MODIFY ";
	}
	if (event->mask & IN_ACCESS) {
		resultString += "ACCESS ";
	}
	if (event->mask & IN_ATTRIB) {
		resultString += "ATTRIB ";
	}
	if (event->mask & IN_CLOSE_WRITE) {
		resultString += "CLOSE_WRITE ";
	}
	if (event->mask & IN_CLOSE_NOWRITE) {
		resultString += "CLOSE_NOWRITE ";
	}
	if (event->mask & IN_DELETE) {
		resultString += "DELETE ";
	}
	if (event->mask & IN_DELETE_SELF) {
		resultString += "DELETE_SELF ";
	}
	if (event->mask & IN_MOVE_SELF) {
		resultString += "MOVE_SELF ";
	}
	if (event->mask & IN_MOVED_FROM) {
		resultString += "MOVED_FROM ";
	}
	if (event->mask & IN_MOVED_TO) {
		resultString += "MOVED_TO ";
	}
	if (event->mask & IN_OPEN) {
		resultString += "OPEN ";
	}
	if (event->mask & IN_ALL_EVENTS) {
		resultString += "ALL_EVENTS ";
	}
	if (event->mask & IN_MOVE) {
		resultString += "MOVE ";
	}
	if (event->mask & IN_CLOSE) {
		resultString += "CLOSE ";
	}
	if (event->mask & IN_DONT_FOLLOW) {
		resultString += "DONT_FOLLOW ";
	}
	if (event->mask & IN_MASK_ADD) {
		resultString += "MASK_ADD ";
	}
	if (event->mask & IN_ONESHOT) {
		resultString += "ONESHOT ";
	}
	if (event->mask & IN_ONLYDIR) {
		resultString += "ONLYDIR ";
	}
	if (event->mask & IN_IGNORED) {
		resultString += "IGNORED ";
	}
	if (event->mask & IN_ISDIR) {
		resultString += "ISDIR ";
	}
	if (event->mask & IN_Q_OVERFLOW) {
		resultString += "Q_OVERFLOW ";
	}
	if (event->mask & IN_UNMOUNT) {
		resultString += "UNMOUNT ";
	}
	resultString += "\n";
	return resultString.c_str();
}

static void handleSignal(int sign) {
	if (sign == SIGINT || sign == SIGSTOP) {
		syslog(LOG_NOTICE, "Stopping daemon");
		closelog();
		exit(EXIT_SUCCESS);
		running = false;
		signal(SIGINT, SIG_DFL);
	} else if (sign == SIGCHLD) {
		cerr << endl;
	}
}

static void listDir(const char* dirName, int& inotifyFD, int& watch) {
	DIR* d;
	d = opendir (dirName);
	if (!d) {
		fprintf (stderr, "Cannot open directory '%s': %s\n",
		         dirName, strerror (errno));
		exit (EXIT_FAILURE);
	}
	while (1) {
		struct dirent* entry;
		const char* d_name;
		entry = readdir (d);
		if (!entry) {
			break;
		}
		d_name = entry->d_name;
		if (entry->d_type & DT_DIR) {
			if (strcmp (d_name, "..") != 0 &&
			    strcmp (d_name, ".") != 0) {
				int path_length;
				char path[PATH_MAX];
				path_length = snprintf (path, PATH_MAX,
				                        "%s/%s", dirName, d_name);
				if (path_length >= PATH_MAX) {
					fprintf (stderr, "Path length has got too long.\n");
					exit (EXIT_FAILURE);
				}
				cout << "Path: " << path << endl;
				// Añadir un WATCH al directorio
				watch = inotify_add_watch(inotifyFD, path, IN_ALL_EVENTS);
				if (watch == -1) {
					printf("Error");
					exit(1);
				}
				listDir (path, inotifyFD, watch);
			}
		}
	}
	if (closedir (d)) {
		fprintf (stderr, "Could not close '%s': %s\n",
		         dirName, strerror (errno));
		exit (EXIT_FAILURE);
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
	syslog(LOG_NOTICE, "Successfully started DAEMON");
	signal(SIGCHLD, SIG_IGN);
	signal(SIGINT, handleSignal);
	signal(SIGSTOP, handleSignal);
}

static int connectToServer(const char* serverID, const int port) {
	/**
	 * Conexion con el servidor
	 */
	int sockfd, i;
	struct hostent* server;
	struct sockaddr_in servAddr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cerr << "Error creating socket file descriptor" << endl;
		exit(-1);
	}
	server = gethostbyname(serverID); // Dirección del servidor
	if (server == NULL) {
		cerr << "Error trying to get hostbyname for server" << endl;
		exit(-1);
	}
	bzero((char*)& servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)& servAddr.sin_addr.s_addr, server->h_length);
	servAddr.sin_port = htons(port);
	if (connect(sockfd, (struct sockaddr*)& servAddr, sizeof(servAddr)) < 0) {
		cerr << "Errory trying to connect to server" << endl;
		exit(-1);
	}
	// ENVIAMOS PID PARA ID
	// Enviamos el pid para identificar al cliente
	long pid = (long) getpid();
	ostringstream ss;
	ss << pid;
	string pidStr = ss.str();
	send(sockfd, pidStr.c_str(), sizeof(pidStr.c_str()), 0);
	return sockfd;
}

static void doWork(int& sockfd, int argc, char* argv[]) {
	struct inotify_event* event;
	int inotifyFD, watch;
	char buf[BUF_LEN];
	ssize_t numRead;
	char* buffer;
	char response[256];
	bool inDir = false;
	// EMPEZAMOS A REGISTRAR EVENTOS
#ifdef DEBUG
	cout << "Starting to reg" << endl;
#endif
	inotifyFD = inotify_init();
	if (inotifyFD == -1) {
		printf("Error");
		exit(1);
	}
	for (int i = 1; i < argc - 2; i++) {
#ifdef DEBUG
		cout << "Path: " << argv[i] << endl;
#endif
		watch = inotify_add_watch(inotifyFD, argv[i], IN_ALL_EVENTS);
		listDir(argv[i], inotifyFD, watch);
		if (watch == -1) {
			printf("Error");
			exit(1);
		}
	}
	for ( ; ; ) {
		numRead = read(inotifyFD, buf, BUF_LEN);
		for (buffer = buf; buffer < buf + numRead; ) {
			event = (struct inotify_event*) buffer;
			const char* bufferServer = showEvent(event, inDir);
			int i = write(sockfd, bufferServer, strlen(bufferServer));
			// Leemos la confirmacion
			bzero(response, 256);
			i = read(sockfd, response, 255);
			if (i > 0) {
				syslog(LOG_NOTICE, response);
			}
			buffer += sizeof(struct inotify_event) + event->len;
		}
		/**
		 * Si se produce un evento dentro un un directorio comprobamos si se han  * creado o borrado nuevos subdirectorios
		 */
		if (inDir) {
			listDir(argv[1], inotifyFD, watch);
			inDir = false;
		}
	}
}


int main (int argc, char* argv[]) {
	if (argc != NUM_ARGS) {
		cerr << "Error in args.\nUsage: ./daemon.o <path> <server> <port>" << endl;
		exit(EXIT_SUCCESS);
	}
	daemonize();
	int sockfd = connectToServer(argv[2], atoi(argv[3]));
	doWork(sockfd, argc, argv);
	return (EXIT_SUCCESS);
}