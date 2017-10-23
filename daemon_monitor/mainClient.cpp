

/**
 * Como argumentos recibe el nombre del servidor (localhost en nuestro caso)
 * el número de puerto en el que debe realizar la conexión y la ruta a
 * controlar
 *
 **/


#include "Client.hpp"
#include <vector>

using namespace std;


int main(int argc, char const* argv[]) {
	vector<char const*> copy;
	for (int i = 3; i < argc; i++) {
		copy.push_back(argv[i]);
	}
	Client client(argv[1], atoi(argv[2]), (argc - 2), copy);
	return 0;
}