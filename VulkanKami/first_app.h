#pragma once

#include "vkm_window.h"

namespace vkm {
	class FirstApp {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
	
		void run();
	private:
		VkmWindow vkmWindow{WIDTH, HEIGHT, "hello Vulkan!"};
	};
} // Namespace vkm