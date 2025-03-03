#pragma once

#include "vkm_game_object.h"
#include "vkm_window.h"

namespace vkm {
	// Not prefixed with VKM due to this class not staying as a core feature of the engine at this time.
	class KeyboardMovementController { 
	public:
		struct KeyMappings {
			int moveLeft = GLFW_KEY_A; // WASD movement
			int moveRight = GLFW_KEY_D;
			int moveForward = GLFW_KEY_W;
			int moveBackward = GLFW_KEY_S;
			int moveUp = GLFW_KEY_E; // E, Q for Up/Down
			int moveDown = GLFW_KEY_Q;
			int lookLeft = GLFW_KEY_LEFT; // Arrow Keys for camera movement
			int lookRight = GLFW_KEY_RIGHT;
			int lookUp = GLFW_KEY_UP;
			int lookDown = GLFW_KEY_DOWN;
		};

		// Works by moving game objects with controls relative to where the object is facing relative to the XZ plane (Optional direction)
		// NOTE THAT THIS IS DEPENDENT ON GLFW, IF DESIRING TO USE OTHER WINDOWING SYSTEMS YOU WILL HAVE TO REWRITE THIS

		void moveInPlaneXZ(GLFWwindow* window, float dt, VkmGameObject& gameObject);

		KeyMappings keys{};
		// Determines speed of moving objects and *camera*
		float moveSpeed{ 3.f };
		float lookSpeed{ 1.5f };
	};
}
