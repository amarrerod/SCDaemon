

#include "monitor.hpp"
#include "processID.hpp"

const int Monitor::BUFFER_LENGTH = (10 * (sizeof(struct inotify_event) + NAME_MAX + 1));

const string Monitor::MONITOR = "monitor";

Monitor::Monitor():
	inotifyFileDescriptor(0),
	eventWatch(0) {}

Monitor::~Monitor() {}


string Monitor::showEvent(struct inotify_event* event) {
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
	return resultString;
}


/**
 * Método encargado de inicializar el demonio
 * Creamos el demonio (PID)
 * Matamos al padre
 * Asignamos la nueva máscara
 * Lo hacemos lider de sesión (SID)
 **/
void Monitor::start() {
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
}

/**
 * Método que se encarga de parar la ejecución del demonio.
 * En primer lugar obtenemos el PID del proceso en ejecución
 * y posteriormente lo eliminamos con Kill
 **/
void Monitor::stop() {
	pid_t currentPID = getProcIdByName(Monitor::MONITOR);
	if (currentPID != -1) {
		// Kill the process
		if (kill(currentPID, -9) < 0) {
			cerr << "Error trying to kill the process with PID: " << currentPID << endl;
			exit(1);
		} else {
			exit(0);
		}
	} else {
		cerr << "Error: Monitor daemon was not running" << endl;
		exit (1);
	}
}


/**
 * Método que se encarga de reiniciar la ejecución del demonio
 * Invocamos a STOP y luego a START
 **/
void Monitor::restart() {
	stop();
	start();
}

/**
 * Método principal de trabajo del demonio
 **/
void Monitor::run(const int arguments, const char* pathname[]) {
	while (1) {
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
				cout << showEvent(event);
				buffer += sizeof(struct inotify_event*) + event->len;
			}
		}
	}
}