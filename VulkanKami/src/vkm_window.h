#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vkm {

	class VkmWindow {

	public:
		VkmWindow(int w, int h, std::string name);
		~VkmWindow();

		VkmWindow(const VkmWindow &) = delete;
		VkmWindow &operator=(const VkmWindow &) = delete;

		bool shouldClose() { return glfwWindowShouldClose(window); } // Wrapper function
	private:
		void initWindow();

		const int width;
		const int height;

		std::string windowName;
		GLFWwindow *window;
	};

} // Namespace vkm