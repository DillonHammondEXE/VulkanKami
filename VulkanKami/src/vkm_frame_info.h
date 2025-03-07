#pragma once

#include "vkm_camera.h"

// lib
#include <vulkan/vulkan.h>

namespace vkm {
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		VkmCamera &camera;
		VkDescriptorSet globalDescriptorSet;
	};

}
