#pragma once

#include "vkm_device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace vkm {

	class VkmDescriptorSetLayout {
	public:
		class Builder {
		public:
			Builder(VkmDevice &vkmDevice) : vkmDevice{ vkmDevice } {}

			Builder &addBinding(
				uint32_t binding,
				VkDescriptorType descriptorType,
				VkShaderStageFlags stageFlags,
				uint32_t count = 1);
			std::unique_ptr<VkmDescriptorSetLayout> build() const;

		private:
			VkmDevice &vkmDevice;
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
		};

		VkmDescriptorSetLayout(
			VkmDevice &vkmDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
		~VkmDescriptorSetLayout();
		VkmDescriptorSetLayout(const VkmDescriptorSetLayout &) = delete;
		VkmDescriptorSetLayout &operator=(const VkmDescriptorSetLayout &) = delete;

		VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

	private:
		VkmDevice &vkmDevice;
		VkDescriptorSetLayout descriptorSetLayout;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

		friend class VkmDescriptorWriter;
	};


	class VkmDescriptorPool {
	public:
		class Builder {
		public:
			Builder(VkmDevice &vkmDevice) : vkmDevice{ vkmDevice } {}

			Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
			Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
			Builder &setMaxSets(uint32_t count);
			std::unique_ptr<VkmDescriptorPool> build() const;

		private:
			VkmDevice &vkmDevice;
			std::vector<VkDescriptorPoolSize> poolSizes{};
			uint32_t maxSets = 1000;
			VkDescriptorPoolCreateFlags poolFlags = 0;
		};

		VkmDescriptorPool(
			VkmDevice &vkmDevice,
			uint32_t maxSets,
			VkDescriptorPoolCreateFlags poolFlags,
			const std::vector<VkDescriptorPoolSize> &poolSizes);
		~VkmDescriptorPool();
		VkmDescriptorPool(const VkmDescriptorPool &) = delete;
		VkmDescriptorPool &operator=(const VkmDescriptorPool &) = delete;

		bool allocateDescriptor(
			const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

		void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

		void resetPool();

	private:
		VkmDevice &vkmDevice;
		VkDescriptorPool descriptorPool;

		friend class VkmDescriptorWriter;
	};

	class VkmDescriptorWriter {
	public:
		VkmDescriptorWriter(VkmDescriptorSetLayout &setLayout, VkmDescriptorPool &pool);

		VkmDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
		VkmDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

		bool build(VkDescriptorSet &set);
		void overwrite(VkDescriptorSet &set);

	private:
		VkmDescriptorSetLayout &setLayout;
		VkmDescriptorPool &pool;
		std::vector<VkWriteDescriptorSet> writes;
	};

}  // namespace vkm

