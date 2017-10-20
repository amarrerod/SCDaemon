

/**
 * Argumentos:
 * 1. Tipo de ejecucion: Start, Stop, Restart
 * 2. Directorio de seguimiento
 *
 */

#include "daemon/monitor.hpp"
#include <iostream>

using namespace std;

const int NUM_ARGS = 1;

int main(int argc, char const* argv[]) {
	if (argc <= NUM_ARGS) {
		cerr << "Error in args" << endl;
		cerr << "Usage: monitor.o <path>" << endl;
		exit(EXIT_SUCCESS);
	} else {
		Monitor monitor;
		monitor.start(argc, argv);
	}
	return 0;
}
