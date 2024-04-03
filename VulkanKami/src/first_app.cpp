#include "first_app.h"

namespace vkm {

	void FirstApp::run() {

		while (!vkmWindow.shouldClose()) {
			glfwPollEvents();
		}
	}





}