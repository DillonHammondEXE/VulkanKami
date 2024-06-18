#include "vkm_renderer.h"

// Standard Libraries
#include <stdexcept>
#include <cassert>
#include <array>

namespace vkm {

	VkmRenderer::VkmRenderer(VkmWindow &window, VkmDevice& device) : vkmWindow{ window }, vkmDevice{ device } {
		recreateSwapChain();
		createCommandBuffers();
	}

	VkmRenderer::~VkmRenderer() { freeCommandBuffers(); }


	void VkmRenderer::recreateSwapChain() {
		auto extent = vkmWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) { // This occurs when minimizing the window
			extent = vkmWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(vkmDevice.device());
		if (vkmSwapChain == nullptr) {
			vkmSwapChain = std::make_unique<VkmSwapChain>(vkmDevice, extent);
		}
		else {
			vkmSwapChain = std::make_unique<VkmSwapChain>(vkmDevice, extent, std::move(vkmSwapChain));
			if (vkmSwapChain->imageCount() != commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}


		// if render pass is compatible then we can do nothing here
		// WELL BE BACK HERE SOON!!!!!!!!!!!!
	}

	void VkmRenderer::createCommandBuffers() {

		commandBuffers.resize(vkmSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		// Creating and destroying command buffers is a freqeunt operation
		// Therefore we use a command pool and can free memory once
		allocInfo.commandPool = vkmDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(vkmDevice.device(), &allocInfo, commandBuffers.data()) !=
			VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers!");
		}

	}

	void VkmRenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(vkmDevice.device(),
			vkmDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data()
		);
		commandBuffers.clear();
	}


	VkCommandBuffer VkmRenderer::beginFrame() {
		assert(!isFrameStarted && "Can't call beginFrame while already in progress");
	
		auto result = vkmSwapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			// Need to handle because it can occur during window resize
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer!");
		}
		return commandBuffer;
	}
	void VkmRenderer::endFrame() {
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer!");
		}

		auto result = vkmSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vkmWindow.wasWindowResized()) {
			vkmWindow.resetWindowResizedFlag();
			recreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image!");
		}

		isFrameStarted = false;
	}
	void VkmRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
	
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() &&
			"Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vkmSwapChain->getRenderPass();
		renderPassInfo.framebuffer = vkmSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vkmSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		// Index 0 is color, Index 1 is depth
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f }; // Background color
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		// VK_SUBPASS_CONTENTS_INLINE (Render pass commands only in primary not secondary)
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(vkmSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(vkmSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, vkmSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void VkmRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() &&
			"Can't end render pass on command buffer from a different frame");
		vkCmdEndRenderPass(commandBuffer);
	}

} // Namespace vkm