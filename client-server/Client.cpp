


/**
 * Como argumentos recibe el nombre del servidor (localhost en nuestro caso)
 * y el número de puerto en el que debe realizar la conexión
 *
 **/

#include "Client.hpp"
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
#include <iomanip>
#include <sstream>

using namespace std;

Client::Client() { }

Client::~Client() { }

Client::Client(char const* host, int port) {
	portNumber = port;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cerr << "Error creating socket file descriptor" << endl;
		exit(-1);
	}
	server = gethostbyname(host); // Dirección del servidor
	if (server == NULL) {
		cerr << "Error trying to get hostbyname for server" << endl;
		exit(-1);
	}
	bzero((char*)& servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)& servAddr.sin_addr.s_addr, server->h_length);
	servAddr.sin_port = htons(portNumber);
	if (connect(sockfd, (struct sockaddr*)& servAddr, sizeof(servAddr)) < 0) {
		cerr << "Errory trying to connect to server" << endl;
		exit(-1);
	}
}

void Client::run() {
	// Enviamos el pid para identificar al cliente
	long pid = (long) getpid();
	ostringstream ss;
	ss << pid;
	string pidStr = ss.str();
	send(sockfd, pidStr.c_str(), sizeof(pidStr.c_str()), 0);
	bool finish = false;
	i = read(sockfd, buffer, 255); // Leemos la confirmacion
	if (i < 0) {
		cerr << "Error trying to read from buffer" << endl;
	} else {
		cout << "Server: " << buffer << endl;
	}
	bzero(buffer, 256);
	while (!finish) {
		cout << "Introduzca el mensaje: \n> ";
		bzero(buffer, 256);
		fgets(buffer, 255, stdin);
		string str(buffer, 4); // Posible comando de salida
		i = write(sockfd, buffer, strlen(buffer));
		if (i < 0) {
			cerr << "Couldn't be able to write in the socket" << endl;
		}
		// Leemos la confirmacion
		bzero(buffer, 256);
		i = read(sockfd, buffer, 255);
		if (i < 0) {
			cerr << "Couldn't be able to read from the socket" << endl;
		} else {
			cout << "Server: " << buffer << endl;
		}
		if (str.compare("exit") == 0) {
			finish = true;
			exit(0);
		}
	}
	close(sockfd);
}


