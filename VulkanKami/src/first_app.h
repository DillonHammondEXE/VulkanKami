#pragma once

#include "vkm_window.h"
#include "vkm_pipeline.h"
#include "vkm_device.h"
#include "vkm_game_object.h"
#include "vkm_swap_chain.h"
// #include "vkm_model.h"

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
		void loadGameObjects();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void freeCommandBuffers();
		void drawFrame();
		void recreateSwapChain();
		void recordCommandBuffer(int imageIndex);
		void renderGameObjects(VkCommandBuffer commandBuffer);
		void sierpinski(
			std::vector<VkmModel::Vertex> &vertices,
			int depth,
			glm::vec2 left,
			glm::vec2 right,
			glm::vec2 top);
		void kochSnowflake(
			std::vector<VkmModel::Vertex>& vertices,
			int depth,
			glm::vec2 start,
			glm::vec2 end,
			float lineWidth);
		// ORDER HERE MATTERS
		VkmWindow vkmWindow{WIDTH, HEIGHT, "VulkanKami"};
		VkmDevice vkmDevice{ vkmWindow };
		std::unique_ptr<VkmSwapChain> vkmSwapChain;
		std::unique_ptr<VkmPipeline> vkmPipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<VkmGameObject> gameObjects;

	};
} // Namespace vkm