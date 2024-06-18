#pragma once

#include "vkm_window.h"
#include "vkm_device.h"
#include "vkm_swap_chain.h"
// #include "vkm_model.h"

// Standard Library
#include <cassert>
#include <memory>
#include <vector>


namespace vkm {
	class VkmRenderer {

	public:

		VkmRenderer(VkmWindow &window, VkmDevice &device);
		~VkmRenderer();

		VkmRenderer(const VkmWindow &) = delete;
		VkmRenderer &operator=(const VkmWindow &) = delete;

		VkRenderPass getSwapChainRenderPass() const { return vkmSwapChain->getRenderPass(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		// ORDER HERE MATTERS
		VkmWindow& vkmWindow;
		VkmDevice& vkmDevice;
		std::unique_ptr<VkmSwapChain> vkmSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{ 0 };
		bool isFrameStarted{ false }; // Should be initially false?
	};
} // Namespace vkm