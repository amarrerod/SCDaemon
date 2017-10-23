

/**
 * Como argumentos recibe el nombre del servidor (localhost en nuestro caso)
 * y el número de puerto en el que debe realizar la conexión
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/inotify.h>
#include <limits.h>
#include <iostream>
#include <cstdlib>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <string>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <fstream>
#include <signal.h>

using namespace std;

class Client {
 private:
	int sockfd, portNumber, i;
	struct hostent* server;
	struct sockaddr_in servAddr;
	pid_t pid, sid;
	int inotifyFileDescriptor;
	int eventWatch;
	int lock;
	struct inotify_event* event;
 public:
	Client();
	Client(char const* host, int port, const int arguments, vector<const char*> pathname);
	virtual ~Client();
	void run();
	const char* showEvent(struct inotify_event* event);
	void start(const int arguments, vector<const char*> pathname);
	void run(const int, vector<const char*> pathname);
 private:
	static void handleSignal(int signal);
 public:
	static const int BUFFER_LENGTH;
	static const string CLIENT;
};
