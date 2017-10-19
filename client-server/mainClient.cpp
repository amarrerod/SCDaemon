
#include "Client.hpp"


int main(int argc, char const* argv[]) {
	Client client(argv[1], atoi(argv[2]));
	client.run();
	return 0;
}