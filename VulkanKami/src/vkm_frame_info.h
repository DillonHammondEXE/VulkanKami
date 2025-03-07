#pragma once

#include "vkm_camera.h"
#include "vkm_game_object.h"

// lib
#include <vulkan/vulkan.h>

namespace vkm {
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		VkmCamera &camera;
		VkDescriptorSet globalDescriptorSet;
		VkmGameObject::Map &gameObjects; // Any system can access all active game objects
	};

}
