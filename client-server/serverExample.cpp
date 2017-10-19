
/**
 * Como argumentos recibe el número de puerto en el que debe realizar la
 * conexión
 *
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;


int main(int argc, char const* argv[]) {
	int sockfd, newSockFd, portNumber;
	socklen_t clientLength;
	char buffer[256];
	struct sockaddr_in servAddr, clientAddr;
	int i;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cerr << "Error creating socket file descriptor" << endl;
		exit(-1);
	}
	bzero((char*)& servAddr, sizeof(servAddr));
	portNumber = atoi(argv[1]); // Número de puerto como parámetro de ejecución
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(portNumber);
	if (bind(sockfd, (struct sockaddr*)& servAddr, sizeof(servAddr)) < 0) {
		cerr << "Error trying to bind" << endl;
		exit(-1);
	}
	listen(sockfd, 5);
	clientLength = sizeof(clientAddr);
	newSockFd = accept(sockfd, (struct sockaddr*)& clientAddr, &clientLength);
	if (newSockFd < 0) {
		cerr << "Error creating socket file descriptor for client" << endl;
		exit(-1);
	}
	bzero(buffer, 256);
	i = read(newSockFd, buffer, 255);
	if (i < 0) {
		cerr << "Error trying to read from buffer. I = " << i << endl;
	} else {
		cout << "Received message: " << buffer << endl;
	}
	close(newSockFd);
	close(sockfd);
	return 0;
}