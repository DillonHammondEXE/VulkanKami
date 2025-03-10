#include "point_light_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// Standard Libraries
#include <stdexcept>
#include <array>

namespace vkm {

	struct PointLightPushConstants {
		glm::vec4 position{};
		glm::vec4 color{};
		float radius;
	};

	PointLightSystem::PointLightSystem(
		VkmDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) 
		: vkmDevice{ device } {
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	PointLightSystem::~PointLightSystem() {
		vkDestroyPipelineLayout(vkmDevice.device(), pipelineLayout, nullptr);
	}


	void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PointLightPushConstants);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(vkmDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
			VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout!");
		}
	}

	void PointLightSystem::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		// auto pipelineConfig =
			// VkmPipeline::defaultPipelineConfigInfo(vkmSwapChain->width(), vkmSwapChain->height());
		PipelineConfigInfo pipelineConfig{};
		VkmPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.attributeDescriptions.clear();
		pipelineConfig.bindingDescriptions.clear();
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		vkmPipeline = std::make_unique<VkmPipeline>(
			vkmDevice,
			"src/shaders/point_light.vert.spv",
			"src/shaders/point_light.frag.spv",
			pipelineConfig);
	}

	/* void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo &ubo) {
		auto rotateLight = glm::rotate(glm::mat4(1.f), frameInfo.frameTime,{ 0.f, -1.f, 0.f });

		int lightIndex = 0;
		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (obj.pointLight == nullptr) continue;

			assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

			// Update light position
			obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));


			// Copy light to ubo
			ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
			ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);

			lightIndex += 1;
		}
		ubo.numLights = lightIndex;
	} */
	// Advanced orbital system with elliptical paths and inclinations
	void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo &ubo) {
		static float totalTime = 0.0f;
		totalTime += frameInfo.frameTime * 8;

		int lightIndex = 0;
		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (obj.pointLight == nullptr) continue;

			assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

			// Define unique orbital parameters for each light
			const float orbitSpeed = 0.5f + (lightIndex * 0.15f);
			const float orbitPeriod = glm::two_pi<float>() / orbitSpeed;
			const float currentAngle = totalTime / orbitPeriod;

			// Orbital parameters
			const float a = 2.0f + (lightIndex * 0.1f);  // Semi-major axis
			const float b = 1.5f + (lightIndex * 0.1f);  // Semi-minor axis
			const float inclination = glm::radians(15.0f + (lightIndex * 5.0f)); // Orbit inclination

			// Calculate elliptical orbit position
			const float x = a * cos(currentAngle + lightIndex * 0.5f);
			const float z = b * sin(currentAngle + lightIndex * 0.5f);

			// Apply inclination rotation around X axis
			glm::mat4 inclinationMatrix = glm::rotate(glm::mat4(1.0f), inclination, glm::vec3(1.0f, 0.0f, 0.0f));

			// Apply precession (slow rotation of the orbital plane)
			float precessionSpeed = 0.1f;
			glm::mat4 precessionMatrix = glm::rotate(glm::mat4(1.0f), totalTime * precessionSpeed, glm::vec3(0.0f, 1.0f, 0.0f));

			// Combine transformations
			glm::vec4 position = precessionMatrix * inclinationMatrix * glm::vec4(x, 0.0f, z, 1.0f);

			// Update light position
			obj.transform.translation = glm::vec3(position);
			obj.transform.translation.y -= 1;

			// Calculate distance-based intensity variations
			float distanceFromCenter = glm::length(obj.transform.translation);
			float intensityFactor = 1.0f / (0.8f + 0.2f * distanceFromCenter); // Brighter when closer

			// Add pulsing effect unique to each light
			float pulseSpeed = 2.0f + lightIndex * 0.5f;
			float pulseAmount = 0.2f;
			float pulseFactor = 1.0f + pulseAmount * sin(totalTime * pulseSpeed);

			// Change light color slightly based on position
			glm::vec3 colorVariation = obj.color;
			// Subtle color temperature shift based on height
			float temperatureFactor = 0.1f * sin(totalTime + lightIndex);
			colorVariation.r = glm::clamp(colorVariation.r + temperatureFactor, 0.0f, 1.0f);
			colorVariation.b = glm::clamp(colorVariation.b - temperatureFactor, 0.0f, 1.0f);

			// Copy light data to UBO
			ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
			ubo.pointLights[lightIndex].color = glm::vec4(
				colorVariation,
				obj.pointLight->lightIntensity * intensityFactor * pulseFactor
			);

			lightIndex += 1;
		}
		ubo.numLights = lightIndex;
	}


	void PointLightSystem::render(FrameInfo& frameInfo) {
		vkmPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frameInfo.globalDescriptorSet,
			0, 
			nullptr);

		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (obj.pointLight == nullptr) continue;

			PointLightPushConstants push{};
			push.position = glm::vec4(obj.transform.translation, 1.f);
			push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
			push.radius = obj.transform.scale.x;

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstants),
				&push
			);


			vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
		}
	}

} // Namespace vkm