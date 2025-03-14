#include "first_app.h"

#include "keyboard_movement_controller.h"
#include "vkm_buffer.h"
#include "vkm_camera.h"
#include "simple_render_system.h"
#include "point_light_system.h"
#include "vkm_texture.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <chrono>
#include <cassert>
#include <stdexcept>
#include <numeric>


namespace vkm {

	FirstApp::FirstApp() {
		globalPool =
			VkmDescriptorPool::Builder(vkmDevice)
			.setMaxSets(VkmSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkmSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkmSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		loadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run() {
		std::vector<std::unique_ptr<VkmBuffer>> uboBuffers(VkmSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<VkmBuffer>(
				vkmDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT); // | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT left out for now to demonstrate
			uboBuffers[i]->map();
		}

		auto globalSetLayout = VkmDescriptorSetLayout::Builder(vkmDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		Texture texture = Texture(vkmDevice, "X:\\Vulkan\\VulkanKami\\VulkanKami\\VulkanKami\\src\\textures\\Geomtry.png");

		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = texture.getSampler();
		imageInfo.imageView = texture.getImageView();
		imageInfo.imageLayout = texture.getImageLayout();


		std::vector<VkDescriptorSet> globalDescriptorSets(VkmSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			VkmDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1, &imageInfo)
				.build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem{ 
			vkmDevice, 
			vkmRenderer.getSwapChainRenderPass(), 
			globalSetLayout->getDescriptorSetLayout() };
		PointLightSystem pointLightSystem{
			vkmDevice,
			vkmRenderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout() };
		VkmCamera camera{};

		auto viewerObject = VkmGameObject::createGameObject(); // Has no model and is just used to store camera's state
		viewerObject.transform.translation.z = -2.5f;
		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		while (!vkmWindow.shouldClose()) {
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			//	frameTime = glm::min(frameTime, MAX_FRAME_TIME); // Optional frame limit with predefined constant

			cameraController.moveInPlaneXZ(vkmWindow.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = vkmRenderer.getAspectRatio(); // Used to maintain gameObjs dimensions even when the window is stretched
			// camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1); 
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f); // 45-60 is common range, anyhting vertices outside the near and far range are CLIPPED

			if (auto commandBuffer = vkmRenderer.beginFrame()) {
				int frameIndex = vkmRenderer.getFrameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					gameObjects
				};

				// Update
				GlobalUbo ubo{};
				ubo.projection = camera.getProjection();
				ubo.view = camera.getView();
				ubo.inverseView = camera.getInverseView();
				pointLightSystem.update(frameInfo, ubo);
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// Render
				vkmRenderer.beginSwapChainRenderPass(commandBuffer);

				// Order here matters
				simpleRenderSystem.renderGameObjects(frameInfo);
				pointLightSystem.render(frameInfo);

				vkmRenderer.endSwapChainRenderPass(commandBuffer);
				vkmRenderer.endFrame();
			}
		}
		
		vkDeviceWaitIdle(vkmDevice.device());
	}
	

	void FirstApp::loadGameObjects() {
		std::shared_ptr<VkmModel> vkmModel = VkmModel::createModelFromFile(vkmDevice,
			"X:\\Vulkan\\VulkanKami\\VulkanKami\\VulkanKami\\src\\models\\flat_vase.obj");

		auto gameObj = VkmGameObject::createGameObject();
		gameObj.model = vkmModel;
		gameObj.transform.translation = { -.5f, .5f, 0.f }; // Bigger Z value, farther away object is
		gameObj.transform.scale = glm::vec3(3.f, 1.5f, 3.f);
		gameObjects.emplace(gameObj.getId(), std::move(gameObj));

		vkmModel = VkmModel::createModelFromFile(vkmDevice,
			"X:\\Vulkan\\VulkanKami\\VulkanKami\\VulkanKami\\src\\models\\smooth_vase.obj");

		auto smoothVase = VkmGameObject::createGameObject();
		smoothVase.model = vkmModel;
		smoothVase.transform.translation = { .5f, .5f, 0.f }; // Bigger Z value, farther away object is
		smoothVase.transform.scale = glm::vec3(3.f, 1.5f, 3.f);
		gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

		vkmModel = VkmModel::createModelFromFile(vkmDevice,
			"X:\\Vulkan\\VulkanKami\\VulkanKami\\VulkanKami\\src\\models\\quad.obj");

		auto floor = VkmGameObject::createGameObject();
		floor.model = vkmModel;
		floor.transform.translation = { 0.f, .5f, 0.f }; // Bigger Z value, farther away object is
		floor.transform.scale = glm::vec3(3.f, 1.f, 3.f);
		gameObjects.emplace(floor.getId(), std::move(floor));

		/* std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},    // Red
			{.1f, .1f, 1.f},    // Blue
			{.1f, 1.f, .1f},    // Green
			{1.f, 1.f, .1f},    // Yellow
			{.1f, 1.f, 1.f},    // Cyan
			{1.f, 1.f, 1.f},    // White
			{1.f, .1f, 1.f},    // Magenta
			{.5f, .5f, 1.f},    // Light blue/periwinkle
			{1.f, .5f, .0f},    // Orange
			{1.f, .5f, .0f},    // Orange
			{.7f, .3f, 1.f}     // Purple
		}; */
		//////////////////////////////////////////
		std::vector<glm::vec3> lightColors{};
		// Generate 30 distinct colors with good variation
		int numOfColors = 100;
		for (int i = 0; i < numOfColors; i++) {
			// Use HSV to RGB conversion for even distribution
			// Vary hue across the full spectrum (0 to 360 degrees)
			float hue = (i * 360.0f) / float(numOfColors);

			// Convert HSV to RGB (simplified conversion)
			float h = hue / 60.0f;
			int hi = static_cast<int>(h) % 6;
			float f = h - static_cast<float>(static_cast<int>(h));

			float v = .25f;  // Value (brightness) always high
			float s = 0.8f;  // Saturation high but not max
			float p = v * (1.0f - s);
			float q = v * (1.0f - f * s);
			float t = v * (1.0f - (1.0f - f) * s);

			glm::vec3 color;
			switch (hi) {
			case 0: color = glm::vec3(v, t, p); break;
			case 1: color = glm::vec3(q, v, p); break;
			case 2: color = glm::vec3(p, v, t); break;
			case 3: color = glm::vec3(p, q, v); break;
			case 4: color = glm::vec3(t, p, v); break;
			case 5: color = glm::vec3(v, p, q); break;
			}

			lightColors.push_back(color);
		}
		//////////////////////////////////////////////////////

		for (int i = 0; i < lightColors.size(); i++) {
			auto pointLight = VkmGameObject::makePointLight(0.2f);
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{ 0.f, -1.f, 0.f });
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}
	}

} // Namespace vkm