#include "first_app.h"

// Standard Libraries
#include <stdexcept>
#include <array>

namespace vkm {

	FirstApp::FirstApp() {
		createPipelineLayout();
		createPipeline();
		createCommandBuffers();
	}

	FirstApp::~FirstApp() {
		vkDestroyPipelineLayout(vkmDevice.device(), pipelineLayout, nullptr);
	}

	void FirstApp::run() {

		while (!vkmWindow.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}
		
		vkDeviceWaitIdle(vkmDevice.device());
	}

	void FirstApp::createPipelineLayout() {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(vkmDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
			VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout!");
		}
	}

	void FirstApp::createPipeline() {
		// auto pipelineConfig =
			// VkmPipeline::defaultPipelineConfigInfo(vkmSwapChain.width(), vkmSwapChain.height());
		PipelineConfigInfo pipelineConfig{};
		VkmPipeline::defaultPipelineConfigInfo(
			pipelineConfig,
			vkmSwapChain.width(),
			vkmSwapChain.height());
		pipelineConfig.renderPass = vkmSwapChain.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		vkmPipeline = std::make_unique<VkmPipeline>(
			vkmDevice,
			"src/shaders/simple_shader.vert.spv",
			"src/shaders/simple_shader.frag.spv",
			pipelineConfig);
	}

	void FirstApp::createCommandBuffers() {

		commandBuffers.resize(vkmSwapChain.imageCount());

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

		for (int i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("Failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = vkmSwapChain.getRenderPass();
			renderPassInfo.framebuffer = vkmSwapChain.getFrameBuffer(i);

			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = vkmSwapChain.getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			// Index 0 is color, Index 1 is depth
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0};
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();
			// VK_SUBPASS_CONTENTS_INLINE (Render pass commands only in primary not secondary)
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkmPipeline->bind(commandBuffers[i]);
			// buffer, Vert count, Instances, firstvert,firstinstance
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to record command buffer!");
			}
		}

	}
	void FirstApp::drawFrame() {
		uint32_t imageIndex;
		auto result = vkmSwapChain.acquireNextImage(&imageIndex);

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			// Need to handle because it can occur during window resize
			throw std::runtime_error("Failed to acquire swap chain image!");
		}
	
		result = vkmSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image!");
		}
	}




} // Namespace vkm