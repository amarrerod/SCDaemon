CC = g++
CFLAGS = -I. -Wall

all: monitor

monitor: main.cpp daemon/monitor.cpp
	g++ -g -std=c++14 -I. -Wall -o monitor main.cpp daemon/monitor.cpp
clean:
		rm *.o