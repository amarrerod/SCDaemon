

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
#include <iostream>
#include <netdb.h>
#include <string>

class Client {
 private:
	int sockfd, portNumber, i;
	struct hostent* server;
	char buffer[256];
	struct sockaddr_in servAddr;
 public:
	Client();
	Client(char const* host, int port);
	virtual ~Client();
	void run();
};