#include "first_app.h"

// Standard libraries
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {

	vkm::FirstApp app{};

	try {
		app.run();
	}
	catch (const std::exception &e) {
	std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
	