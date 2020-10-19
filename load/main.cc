#include "document.hpp"

#include <exception>
#include <iostream>

int main(int argc, char* argv[])
try {
	for (int i = 1; i < argc; ++i) {
		Document doc(argv[i]);
	}
}
catch (const std::exception& e) {
	std::cerr << "fatal: " << e.what() << '\n';
}
