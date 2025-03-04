#pragma once

#include "vkm_model.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace vkm {

	struct TransformComponent {
		glm::vec3 translation{};
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};
		// Using euler angles, I'd like to switch quaternions eventually
		// Matrix: translate * Ry * Rx * Rz * scale transformation
		// Using tait-bryan angles with axis order Y(1), X(2), Z(3)
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};
	// Could use OO or Entity Component System in the future
	class VkmGameObject{
	public:
		using id_t = unsigned int;

		static VkmGameObject createGameObject() {
			static id_t currentId = 0;
			return VkmGameObject{ currentId++ };
		}

		VkmGameObject(const VkmGameObject &) = delete;
		VkmGameObject &operator=(const VkmGameObject &) = delete;
		VkmGameObject(VkmGameObject &&) = default;
		VkmGameObject &operator=(VkmGameObject &&) = default;

		id_t gameId() { return id; }

		std::shared_ptr<VkmModel> model{};
		glm::vec3 color{};
		TransformComponent transform{};
	private:
		VkmGameObject(id_t objId) : id{ objId } {}

		id_t id;
	};
}
