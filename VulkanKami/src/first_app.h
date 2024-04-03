#pragma once

#include "vkm_window.h"
#include "vkm_pipeline.h"

namespace vkm {
	class FirstApp {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
	
		void run();
	private:
		VkmWindow vkmWindow{WIDTH, HEIGHT, "hello Vulkan!"};
		VkmPipeline vkmPipeline{ "src/shaders/simple_shader.vert.spv","src/shaders/simple_shader.frag.spv"  };
	};
} // Namespace vkm