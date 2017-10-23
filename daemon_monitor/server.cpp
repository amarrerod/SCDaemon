
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
#include <string>
#include <chrono>
#include <cstring>


using namespace std;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;

const char* CONNECTION = "Connection sucessfull";
const char* RECEIVED = "Message received";
const int CHRONO = 300;

bool checkChrono(auto& startTime) {
	auto currentTime = chrono::high_resolution_clock::now();
	if (chrono::duration_cast<chrono::seconds>(currentTime - startTime).count() >= CHRONO) {
		// Enviar informacion del log
		startTime = chrono::high_resolution_clock::now();
		return true;
	} else
		return false;
}

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
	auto startTime = chrono::high_resolution_clock::now();
	while (true) {
		newSockFd = accept(sockfd, (struct sockaddr*)& clientAddr, &clientLength);
		if (newSockFd < 0) {
			cerr << "Error creating socket file descriptor for client" << endl;
			exit(-1);
		}
		pid_t pid = fork();
		if (pid < 0) {
			cerr << "Error trying to create a new process" << endl;
			exit(-1);
		}
		if (pid == 0) {
			close(sockfd);
			bzero(buffer, 256);
			bool finish = false;
			string pidStr;
			i = read(newSockFd, buffer, sizeof(long));
			if (i < 0) {
				cerr << "Error trying to read pid from client" << endl;
			} else {
				pidStr = buffer;
			}
			bzero(buffer, 256);
			send(newSockFd, CONNECTION, strlen(CONNECTION), 0); // Avisamos que la conexion ha sido realizada
			while (true) {
				i = read(newSockFd, buffer, 255);
				if (i < 0) {
					cerr << "Error trying to read from buffer. I = " << i << endl;
				} else {
					i += 1;
					// TODO
					// GUARDAR ESTA INFORMACION EN UN FICHERO
					//	cout << "Client(" << pidStr << "): " << buffer << endl;
					//	send(newSockFd, RECEIVED, strlen(RECEIVED), 0); // Enviamos la confirmacion
					//	string str(buffer, 4);
					//	if (str.compare("exit") == 0) {
					//		cout << "Closing connection with the user: " << pidStr << endl;
					//		finish = true;
				}
				bzero(buffer, 256); // Limpiamos el buffer
				if (checkChrono(startTime)) {
					// ENVIAR LOG ALMACENADO EN EL FICHERO
					send(newSockFd, RECEIVED, strlen(RECEIVED), 0);
				}
			}
			exit(0);
		} else {
			close(newSockFd);
		}
	}
	close(sockfd);
	return 0;
}