#pragma once

#include "vkm_window.h"
#include "vkm_device.h"
#include "vkm_game_object.h"
#include "vkm_renderer.h"
// #include "vkm_model.h"

// Standard Library
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

		std::vector<VkmGameObject> gameObjects;

	};
} // Namespace vkm