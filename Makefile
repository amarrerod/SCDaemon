

monitor: main.cpp daemon/monitor.cpp
	g++ -g -o monitor.o main.cpp daemon/monitor.cpp

clean:
	rm *.o