#pragma once

#include "vkm_camera.h"
#include "vkm_device.h"
#include "vkm_frame_info.h"
#include "vkm_pipeline.h"
#include "vkm_game_object.h"

// Standard Library
#include <memory>
#include <vector>

namespace vkm {
	class PointLightSystem {

	public:

		PointLightSystem(VkmDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		PointLightSystem(const VkmWindow &) = delete;
		PointLightSystem &operator=(const VkmWindow &) = delete;

		void render(FrameInfo &frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);
		// ORDER HERE MATTERS
		VkmDevice& vkmDevice;


		std::unique_ptr<VkmPipeline> vkmPipeline;
		VkPipelineLayout pipelineLayout;
	};
} // Namespace vkm
