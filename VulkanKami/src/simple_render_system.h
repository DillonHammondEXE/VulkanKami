#pragma once

#include "vkm_pipeline.h"
#include "vkm_device.h"
#include "vkm_game_object.h"

// Standard Library
#include <memory>
#include <vector>

namespace vkm {
	class SimpleRenderSystem {

	public:

		SimpleRenderSystem(VkmDevice& device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const VkmWindow &) = delete;
		SimpleRenderSystem &operator=(const VkmWindow &) = delete;

		void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<VkmGameObject> &gameObjects);

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);
		// ORDER HERE MATTERS
		VkmDevice& vkmDevice;


		std::unique_ptr<VkmPipeline> vkmPipeline;
		VkPipelineLayout pipelineLayout;
	};
} // Namespace vkm
