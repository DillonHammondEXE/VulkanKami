#pragma once

#include "vkm_model.h"

#include <memory>

namespace vkm {

	struct Transform2dComponent {
		glm::vec2 translation{};
		glm::vec2 scale{ 1.f, 1.f };
		float rotation;

		glm::mat2 mat2() { 
			const float s = glm::sin(rotation);
			const float c = glm::cos(rotation);
			glm::mat2 rotMatrix{ {c, s}, {-s, c} };
			glm::mat2 scaleMat{ {scale.x, .0f}, {.0f, scale.y} }; // TAKES COLUMNS AS PARAMATERS NOT ROWS
			return rotMatrix * scaleMat; 
		}
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
		Transform2dComponent transform2d{};
	private:
		VkmGameObject(id_t objId) : id{ objId } {}

		id_t id;
	};
}
