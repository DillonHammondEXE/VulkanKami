#include "first_app.h"

#include "keyboard_movement_controller.h"
#include "vkm_buffer.h"
#include "vkm_camera.h"
#include "simple_render_system.h"

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

	struct GlobalUbo {
		glm::mat4 projectionView{ 1.f };
		glm::vec3 lightdirection = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
	};

	FirstApp::FirstApp() {
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

		SimpleRenderSystem simpleRenderSystem{ vkmDevice, vkmRenderer.getSwapChainRenderPass() };
		VkmCamera camera{};
		// camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.0f, 2.5f));

		auto viewerObject = VkmGameObject::createGameObject(); // Has no model and is just used to store camera's state
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
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f); // 45-60 is common range, anyhting vertices outside the near and far range are CLIPPED

			if (auto commandBuffer = vkmRenderer.beginFrame()) {
				int frameIndex = vkmRenderer.getFrameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera
				};

				// Update
				GlobalUbo ubo{};
				ubo.projectionView = camera.getProjection() * camera.getView();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// Render
				vkmRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
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
		gameObj.transform.translation = { -.5f, .5f, 2.5f }; // Bigger Z value, farther away object is
		gameObj.transform.scale = glm::vec3(3.f, 1.5f, 3.f);
		gameObjects.push_back(std::move(gameObj));

		vkmModel = VkmModel::createModelFromFile(vkmDevice,
			"X:\\Vulkan\\VulkanKami\\VulkanKami\\VulkanKami\\src\\models\\smooth_vase.obj");

		auto smoothVase = VkmGameObject::createGameObject();
		smoothVase.model = vkmModel;
		smoothVase.transform.translation = { .5f, .5f, 2.5f }; // Bigger Z value, farther away object is
		smoothVase.transform.scale = glm::vec3(3.f, 1.5f, 3.f);
		gameObjects.push_back(std::move(smoothVase));
	}

} // Namespace vkm