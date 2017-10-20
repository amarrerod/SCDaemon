
#ifndef _MONITOR_HPP_
#define _MONITOR_HPP_

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
	pid_t pid, sid;
	int inotifyFileDescriptor;
	int eventWatch;
	int lock;
	struct inotify_event* event;
 public:
	Monitor();
	~Monitor();
	const char* showEvent(struct inotify_event* event);
	void start(const int arguments, const char* pathname[]);
	void run(const int, const char* pathname[]);
 private:
	bool checkChrono(auto startTime);
	static void handleSignal(int signal);
 public:
	static const int BUFFER_LENGTH;
	static const string MONITOR;
	static const int CHRONO;
};

#endif // _MONITOR_HPP_