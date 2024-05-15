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
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		bool wasWindowResized() { return frameBufferResized; }
		void resetWindowResizedFlag() { frameBufferResized = false; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
	private:
		static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
		void initWindow();

		int width;
		int height;
		bool frameBufferResized = false;

		std::string windowName;
		GLFWwindow *window;
	};

} // Namespace vkm