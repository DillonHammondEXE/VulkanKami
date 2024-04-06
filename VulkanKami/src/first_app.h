#pragma once

#include "vkm_window.h"
#include "vkm_pipeline.h"
#include "vkm_device.h"
#include "vkm_swap_chain.h"

// Standard Library
#include <memory>
#include <vector>

namespace vkm {
	class FirstApp {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		FirstApp();
		~FirstApp();

		FirstApp(const VkmWindow &) = delete;
		FirstApp &operator=(const VkmWindow &) = delete;
		void run();

	private:
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();
		// ORDER HERE MATTERS
		VkmWindow vkmWindow{WIDTH, HEIGHT, "hello Vulkan!"};
		VkmDevice vkmDevice{ vkmWindow };
		VkmSwapChain vkmSwapChain{ vkmDevice, vkmWindow.getExtent() };
		std::unique_ptr<VkmPipeline> vkmPipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;

	};
} // Namespace vkm