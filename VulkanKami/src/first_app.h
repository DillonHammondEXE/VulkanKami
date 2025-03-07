#pragma once

#include "vkm_descriptors.h"
#include "vkm_device.h"
#include "vkm_game_object.h"
#include "vkm_renderer.h"
#include "vkm_window.h"

// std
#include <memory>
#include <vector>

namespace vkm {
	class FirstApp {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		FirstApp();
		~FirstApp();

		FirstApp(const VkmWindow &) = delete;
		FirstApp &operator=(const VkmWindow &) = delete;
		void run();

	private:
		void loadGameObjects();

		// ORDER HERE MATTERS
		VkmWindow vkmWindow{WIDTH, HEIGHT, "VulkanKami"};
		VkmDevice vkmDevice{ vkmWindow };
		VkmRenderer vkmRenderer{ vkmWindow, vkmDevice };

		// Note: Order of declarations matters
		std::unique_ptr<VkmDescriptorPool> globalPool{ };
		std::vector<VkmGameObject> gameObjects;

	};
} // Namespace vkm