#include "vkm_window.h"
#include <stdexcept>

namespace vkm {

	VkmWindow::VkmWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}
	VkmWindow::~VkmWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}


	void VkmWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Specify that we aren't using an OpenGL context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Disable resizing (will be handled in a different way)

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
	}

	void VkmWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}
	}
} // Namespace vkm