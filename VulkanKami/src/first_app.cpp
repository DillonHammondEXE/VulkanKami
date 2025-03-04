#include "first_app.h"

#include "keyboard_movement_controller.h"
#include "vkm_camera.h"
#include "simple_render_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// Standard Libraries
#include <array>
#include <chrono>
#include <cassert>
#include <stdexcept>


namespace vkm {

	FirstApp::FirstApp() {
		loadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run() {
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
				vkmRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
				vkmRenderer.endSwapChainRenderPass(commandBuffer);
				vkmRenderer.endFrame();
			}
		}
		
		vkDeviceWaitIdle(vkmDevice.device());
	}
	

	void FirstApp::loadGameObjects() {
		std::shared_ptr<VkmModel> vkmModel = VkmModel::createModelFromFile(vkmDevice, 
			"X:\\Vulkan\\VulkanKami\\VulkanKami\\VulkanKami\\src\\models\\smooth_vase.obj");

		auto gameObj = VkmGameObject::createGameObject();
		gameObj.model = vkmModel;
		gameObj.transform.translation = { .0f, .0f, 2.5f }; // Bigger Z value, farther away object is
		gameObj.transform.scale = glm::vec3(3.f);
		gameObjects.push_back(std::move(gameObj));
	}

} // Namespace vkm