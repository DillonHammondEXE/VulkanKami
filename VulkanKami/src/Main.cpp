#include "first_app.h"

// Standard libraries
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <windows.h> // TESTING DEBUG

int main() {
	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONIN$", "r", stdin);
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);

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
	