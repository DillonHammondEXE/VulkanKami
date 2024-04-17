#include "first_app.h"

// Standard Libraries
#include <stdexcept>
#include <array>

namespace vkm {

	FirstApp::FirstApp() {
		loadModels();
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

	// From Brendan Galea
	void FirstApp::sierpinski(
		std::vector<VkmModel::Vertex> &vertices,
		int depth,
		glm::vec2 left,
		glm::vec2 right,
		glm::vec2 top) {
		if (depth <= 0) {
			vertices.push_back({ top });
			vertices.push_back({ right });
			vertices.push_back({ left });
		}
		else {
			auto leftTop = 0.5f * (left + top);
			auto rightTop = 0.5f * (right + top);
			auto leftRight = 0.5f * (left + right);
			sierpinski(vertices, depth - 1, left, leftRight, leftTop);
			sierpinski(vertices, depth - 1, leftRight, right, rightTop);
			sierpinski(vertices, depth - 1, leftTop, rightTop, top);
		}
	}

	void FirstApp::kochSnowflake(
		std::vector<VkmModel::Vertex>& vertices,
		int depth,
		glm::vec2 start,
		glm::vec2 end,
		float width = 0.01f) {  // width of the line as a very small value
		if (depth == 0) {
			glm::vec2 normal = glm::normalize(glm::vec2(end.y - start.y, -(end.x - start.x))) * width;

			glm::vec2 v1 = start + normal;
			glm::vec2 v2 = start - normal;
			glm::vec2 v3 = end + normal;
			glm::vec2 v4 = end - normal;

			// First triangle
			vertices.push_back({ v1 });
			vertices.push_back({ v2 });
			vertices.push_back({ v3 });

			// Second triangle
			vertices.push_back({ v2 });
			vertices.push_back({ v4 });
			vertices.push_back({ v3 });
		}
		else {
			glm::vec2 vec = end - start;
			glm::vec2 third1 = start + vec / 3.0f;
			glm::vec2 third2 = start + 2.0f * vec / 3.0f;

			// Calculate the point that forms the peak of the "bump"
			float angle = glm::radians(60.0f); // 60 degrees for equilateral triangle
			glm::vec2 peak = third1 + glm::vec2(
				cos(angle) * (third2.x - third1.x) - sin(angle) * (third2.y - third1.y),
				sin(angle) * (third2.x - third1.x) + cos(angle) * (third1.y - third2.y)
			);

			kochSnowflake(vertices, depth - 1, start, third1, width);
			kochSnowflake(vertices, depth - 1, third1, peak, width);
			kochSnowflake(vertices, depth - 1, peak, third2, width);
			kochSnowflake(vertices, depth - 1, third2, end, width);
		}
	}

	void FirstApp::loadModels() {
		/* std::vector<VkmModel::Vertex> vertices;
		glm::vec2 v1 = { -0.5f, -0.3f };
		glm::vec2 v2 = { 0.5f, -0.3f };
		glm::vec2 v3 = { 0.0f, 0.6f };

		float lineWidth = 0.005f;
		kochSnowflake(vertices, 4, v1, v2, lineWidth);
		kochSnowflake(vertices, 4, v2, v3, lineWidth);
		kochSnowflake(vertices, 4, v3, v1, lineWidth); */
		std::vector<VkmModel::Vertex> vertices{
			{ {0.0f, -0.5f}, {1.0f, 0.0f, 0.0f} },
			{ {0.5f, 0.5f},  {0.0f, 1.0f, 0.0f} },
		{ {-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} }};

		vkmModel = std::make_unique<VkmModel>(vkmDevice, vertices);
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
			vkmModel->bind(commandBuffers[i]);
			vkmModel->draw(commandBuffers[i]);

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