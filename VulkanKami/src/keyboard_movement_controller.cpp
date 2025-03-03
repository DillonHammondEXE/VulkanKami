#include "keyboard_movement_controller.h"
#include <limits>

namespace vkm {

	void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, VkmGameObject& gameObject) {

		glm::vec3 rotate{ 0 };
		if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
		if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
		if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
		if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

		// Rotation is normalized to prevent faster speed diagonally
		// Checking if vector is 0 to prevent error when normalizing (is there a better way?)
		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) { // uses epsilon to avoid comparing float with 0
			gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
		}

		// limit pitch values between about +/- 85ish degrees to prevent upside down objects
		gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
		// Prevent repeated spinning which can cause overflow by mod % 360 degrees.
		gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

		// Extract yaw (rotation around the Y-axis) from the object's transform.
		float yaw = gameObject.transform.rotation.y;

		// Compute the forward direction based on yaw:
		// - `sin(yaw)` gives the X component
		// - `cos(yaw)` gives the Z component
		// This forms a unit vector in the XZ plane pointing in the direction the object is facing.
		const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };

		// Compute the right direction, which is perpendicular to the forward direction.
		// - Swapping X and Z and negating one component gives a 90-degree rotated vector.
		// This vector points to the right relative to the object's orientation.
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };

		// The up direction is always constant in this setup, as movement is constrained to the XZ plane.
		// Here, it is set to point downward (`-1.f` in the Y-axis) because of how the coordinate system might be defined (Vulkan -Y is up)'
		// Actually why is -y up?
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };

		glm::vec3 moveDir{ 0.f };
		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir; 
		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) { // uses epsilon to avoid comparing float with 0
			gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		}

	}







}


