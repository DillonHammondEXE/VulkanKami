#include "first_app.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// Standard Libraries
#include <stdexcept>
#include <array>

namespace vkm {

	struct SimplePushConstantData {
		glm::mat2 transform{ 1.f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	FirstApp::FirstApp() {
		loadGameObjects();
		createPipelineLayout();
		recreateSwapChain();
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

	void FirstApp::loadGameObjects() {
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
		auto vkmModel = std::make_shared<VkmModel>(vkmDevice, vertices);

		std::vector<glm::vec3> colors{
			{1.f, .7f, .73f},
			{1.f, .87f, .73f},
			{1.f, 1.f, .73f},
			{.73f, 1.f, .8f},
			{.73, .88f, 1.f}
		};
		for (auto& color : colors) {
			color = glm::pow(color, glm::vec3{ 2.2f });
		}
		for (int i = 0; i < 40; i++) {
			auto triangle = VkmGameObject::createGameObject();
			triangle.model = vkmModel;
			triangle.transform2d.scale = glm::vec2(.5f) + i * 0.025f;
			triangle.transform2d.rotation = i * glm::pi<float>() * .025f;
			triangle.color = colors[i % colors.size()];
			gameObjects.push_back(std::move(triangle));
		}
	}

	void FirstApp::createPipelineLayout() {

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);


		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(vkmDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
			VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout!");
		}
	}

	void FirstApp::createPipeline() {
		assert(vkmSwapChain != nullptr && "Cannot create pipeline before swap chain");
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		// auto pipelineConfig =
			// VkmPipeline::defaultPipelineConfigInfo(vkmSwapChain->width(), vkmSwapChain->height());
		PipelineConfigInfo pipelineConfig{};
		VkmPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = vkmSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		vkmPipeline = std::make_unique<VkmPipeline>(
			vkmDevice,
			"src/shaders/simple_shader.vert.spv",
			"src/shaders/simple_shader.frag.spv",
			pipelineConfig);
	}

	void FirstApp::recreateSwapChain() {
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
		createPipeline();
	}

	void FirstApp::createCommandBuffers() {

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

	void FirstApp::freeCommandBuffers() {
		vkFreeCommandBuffers(vkmDevice.device(),
			vkmDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data()
			);
		commandBuffers.clear();
	}

	void FirstApp::recordCommandBuffer(int imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vkmSwapChain->getRenderPass();
		renderPassInfo.framebuffer = vkmSwapChain->getFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vkmSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		// Index 0 is color, Index 1 is depth
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f }; // Background color
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		// VK_SUBPASS_CONTENTS_INLINE (Render pass commands only in primary not secondary)
		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(vkmSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(vkmSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, vkmSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		renderGameObjects(commandBuffers[imageIndex]);

		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer!");
		}
	}

	void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer) {
		int i = 0;
		for (auto& obj : gameObjects) {
			i += 1;
			obj.transform2d.rotation =
				glm::mod<float>(obj.transform2d.rotation + 0.00005f * i, 2.f * glm::pi<float>());
		}

		vkmPipeline->bind(commandBuffer);

		for (auto& obj : gameObjects) {
			SimplePushConstantData push{};
			push.offset = obj.transform2d.translation;
			push.color = obj.color;
			push.transform = obj.transform2d.mat2();

			vkCmdPushConstants(
				commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
		}
	}


	void FirstApp::drawFrame() {
		uint32_t imageIndex;
		auto result = vkmSwapChain->acquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			// Need to handle because it can occur during window resize
			throw std::runtime_error("Failed to acquire swap chain image!");
		}
		
		recordCommandBuffer(imageIndex);
		result = vkmSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vkmWindow.wasWindowResized()) {
			vkmWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image!");
		}
	}




} // Namespace vkm