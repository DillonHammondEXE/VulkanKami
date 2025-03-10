#pragma once

#include "vkm_camera.h"
#include "vkm_game_object.h"

// lib
#include <vulkan/vulkan.h>

namespace vkm {

	#define MAX_LIGHTS 10

	struct PointLight {
		glm::vec4 position{}; // ignore w
		glm::vec4 color{}; // w is intensity
	};

	// ALWEAYS DOUBLE CHECK ALIGNMENT RULES WHEN MAKING CHANGES TO BUFFER OBJECTS SUCH AS BELOW
	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f }; // W is intensity
		PointLight pointLights[MAX_LIGHTS];
		int numLights;
	};

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		VkmCamera &camera;
		VkDescriptorSet globalDescriptorSet;
		VkmGameObject::Map &gameObjects; // Any system can access all active game objects
	};

}
