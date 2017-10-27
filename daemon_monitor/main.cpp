

/**
 * Como argumentos recibe el nombre del servidor (localhost en nuestro caso)
 * el número de puerto en el que debe realizar la conexión y la ruta a
 * controlar
 *
 **/

#include "Client.hpp"
#include <vector>

using namespace std;

const int NUM_ARGS = 4;

int main(int argc, char const* argv[]) {
	if (argc != NUM_ARGS) {
		cerr << "Error in args.\nUsage: ./daemon.o <server> <port> <path>" << endl;
		exit(EXIT_SUCCESS);
	}
	vector<char const*> copy;
	for (int i = 3; i < argc; i++) {
		copy.push_back(argv[i]);
	}
	for (int i = 0; i < copy.size(); i++)
		cout << "ARG:" << copy[i] << endl;
	Client client(argv[1], atoi(argv[2]), (argc - 2), copy);
	client.run((argc - 2), copy);
	return 0;
}