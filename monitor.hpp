

#include <sys/inotify.h>
#include <limits.h>
#include <iostream>
#include <cstdlib>
#include <sys/types.h>
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

class Monitor {
 private:
	pid_t pid, sid; // PID estático porque solo se puede ejecutar uno
	int inotifyFileDescriptor;
	int eventWatch;
	struct inotify_event* event;
 public:
	Monitor();
	~Monitor();
	string showEvent(struct inotify_event* event);
	void start();
	void stop();
	void restart();
	void run(const int, const char* pathname[]);
 public:
	static const int BUFFER_LENGTH;
	static const string MONITOR;
};