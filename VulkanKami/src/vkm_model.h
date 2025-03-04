#pragma once

#include "vkm_device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


#include <memory>

namespace vkm {
	class VkmModel {
	public:

		struct Vertex {
			glm::vec3 position;
			glm::vec3 color;
			glm::vec3 normal{};
			glm::vec2 uv{};
			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		struct Builder { 
			// Temporary helper object stores vertex and index information 
			// untill it can be copied into the models, vertex, and index buffer memory
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string &filepath);
		};

		VkmModel(VkmDevice& device, const VkmModel::Builder &builder);
		~VkmModel();

		VkmModel(const VkmWindow &) = delete;
		VkmModel &operator=(const VkmWindow &) = delete;

		static std::unique_ptr<VkmModel> createModelFromFile(VkmDevice &device, const std::string &filepath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
	private:
		void createVertexBuffers(const std::vector<Vertex> &vertices);
		void createIndexBuffers(const std::vector<uint32_t> &indices);

		VkmDevice& vkmDevice;

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;

		bool hasIndexBuffer = false; // Allows the choice to just use vertices or vertex AND index buffer
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		uint32_t indexCount;
	};
}
