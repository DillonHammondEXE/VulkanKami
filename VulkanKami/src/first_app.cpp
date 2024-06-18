#include "first_app.h"

#include "simple_render_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// Standard Libraries
#include <stdexcept>
#include <array>

namespace vkm {

	FirstApp::FirstApp() {
		loadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run() {
		SimpleRenderSystem simpleRenderSystem{ vkmDevice, vkmRenderer.getSwapChainRenderPass() };


		while (!vkmWindow.shouldClose()) {
			glfwPollEvents();
			
			if (auto commandBuffer = vkmRenderer.beginFrame()) {
				vkmRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
				vkmRenderer.endSwapChainRenderPass(commandBuffer);
				vkmRenderer.endFrame();
			}
		}
		
		vkDeviceWaitIdle(vkmDevice.device());
	}

	void FirstApp::loadGameObjects() {
		std::vector<VkmModel::Vertex> vertices{
			{ {0.0f, -0.5f}, {1.0f, 0.0f, 0.0f} },
			{ {0.5f, 0.5f},  {0.0f, 1.0f, 0.0f} },
		{ {-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} }};
		auto vkmModel = std::make_shared<VkmModel>(vkmDevice, vertices);

		std::vector<glm::vec3> colors{
			{1.f, .7f, .73f},
			{1.f, .87f, .73f},
			{1.f, 1.f, .73f},
			{.73f, 1.f, .8f},
			{.73, .88f, 1.f}
		};
		for (auto& color : colors) {
			color = glm::pow(color, glm::vec3{ 2.2f });
		}
		for (int i = 0; i < 40; i++) {
			auto triangle = VkmGameObject::createGameObject();
			triangle.model = vkmModel;
			triangle.transform2d.scale = glm::vec2(.5f) + i * 0.025f;
			triangle.transform2d.rotation = i * glm::pi<float>() * .025f;
			triangle.color = colors[i % colors.size()];
			gameObjects.push_back(std::move(triangle));
		}
	}

} // Namespace vkm