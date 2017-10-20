

#include "monitor.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <chrono>
#include <cstring>
#include <sstream>

using namespace std;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;

const int Monitor::BUFFER_LENGTH = (10 * (sizeof(struct inotify_event) + NAME_MAX + 1));
const int Monitor::CHRONO = 300;
const string Monitor::MONITOR = "monitor";
const char* STOPPING = "Stopping daemon";
const char* STARTING = "Successfully started monitor";
const char* LOCK = "monitor.lock";
const char* LOG = "monitor.log";
static bool running = true;
const int DELAY = 1;

Monitor::Monitor():
	inotifyFileDescriptor(0),
	eventWatch(0) {
}

Monitor::~Monitor() {}

bool Monitor::checkChrono(auto startTime) {
	auto currentTime = chrono::high_resolution_clock::now();
	if (chrono::duration_cast<chrono::seconds>(currentTime - startTime).count() >= CHRONO) {
		// Enviar informacion del log
		startTime = chrono::high_resolution_clock::now();
		return true;
	} else
		return false;
}

const char* Monitor::showEvent(struct inotify_event* event) {
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
void Monitor::start(const int arguments, const char* pathname[]) {
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
	syslog(LOG_NOTICE, "Successfully started monitor");
	signal(SIGCHLD, SIG_IGN);
	signal(SIGINT, handleSignal);
	signal(SIGSTOP, handleSignal);
	run(arguments, pathname);
}


void Monitor::handleSignal(int sign) {
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
void Monitor::run(const int arguments, const char* pathname[]) {
	while (running) {
		auto startTime = chrono::high_resolution_clock::now();
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
				syslog(LOG_INFO, showEvent(event));
				buffer += sizeof(struct inotify_event*) + event->len;
			}
		}
		if (checkChrono(startTime)) {
			cout << "ENVIAR LOG" << endl;
		}
		sleep(DELAY);
	}
	// Si salimos del bucle se acaba la ejecucion
	syslog(LOG_NOTICE, "Stopping daemon");
	closelog();
	exit(EXIT_SUCCESS);
}