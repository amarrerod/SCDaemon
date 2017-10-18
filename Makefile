

monitor: main.cpp monitor.cpp
	g++ -g -o monitor.o main.cpp monitor.cpp

clean:
	rm *.o