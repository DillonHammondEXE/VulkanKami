#include <cassert> // Gives error if not included before vkm_descriptors...
#include "vkm_descriptors.h"

// std
#include <cassert>
#include <stdexcept>

namespace vkm {

	// *************** Descriptor Set Layout Builder *********************

	VkmDescriptorSetLayout::Builder &VkmDescriptorSetLayout::Builder::addBinding(
		uint32_t binding,
		VkDescriptorType descriptorType,
		VkShaderStageFlags stageFlags,
		uint32_t count) {
		assert(bindings.count(binding) == 0 && "Binding already in use");
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;
		bindings[binding] = layoutBinding;
		return *this;
	}

	std::unique_ptr<VkmDescriptorSetLayout> VkmDescriptorSetLayout::Builder::build() const {
		return std::make_unique<VkmDescriptorSetLayout>(vkmDevice, bindings);
	}


	// *************** Descriptor Set Layout *********************

	VkmDescriptorSetLayout::VkmDescriptorSetLayout(
		VkmDevice &vkmDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
		: vkmDevice{ vkmDevice }, bindings{ bindings } {
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
		for (auto kv : bindings) {
			setLayoutBindings.push_back(kv.second);
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
		descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

		if (vkCreateDescriptorSetLayout(
			vkmDevice.device(),
			&descriptorSetLayoutInfo,
			nullptr,
			&descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	VkmDescriptorSetLayout::~VkmDescriptorSetLayout() {
		vkDestroyDescriptorSetLayout(vkmDevice.device(), descriptorSetLayout, nullptr);
	}

	// *************** Descriptor Pool Builder *********************

	VkmDescriptorPool::Builder &VkmDescriptorPool::Builder::addPoolSize(
		VkDescriptorType descriptorType, uint32_t count) {
		poolSizes.push_back({ descriptorType, count });
		return *this;
	}

	VkmDescriptorPool::Builder &VkmDescriptorPool::Builder::setPoolFlags(
		VkDescriptorPoolCreateFlags flags) {
		poolFlags = flags;
		return *this;
	}
	VkmDescriptorPool::Builder &VkmDescriptorPool::Builder::setMaxSets(uint32_t count) {
		maxSets = count;
		return *this;
	}

	std::unique_ptr<VkmDescriptorPool> VkmDescriptorPool::Builder::build() const {
		return std::make_unique<VkmDescriptorPool>(vkmDevice, maxSets, poolFlags, poolSizes);
	}

	// *************** Descriptor Pool *********************

	VkmDescriptorPool::VkmDescriptorPool(
		VkmDevice &vkmDevice,
		uint32_t maxSets,
		VkDescriptorPoolCreateFlags poolFlags,
		const std::vector<VkDescriptorPoolSize> &poolSizes)
		: vkmDevice{ vkmDevice } {
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		descriptorPoolInfo.flags = poolFlags;

		if (vkCreateDescriptorPool(vkmDevice.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	VkmDescriptorPool::~VkmDescriptorPool() {
		vkDestroyDescriptorPool(vkmDevice.device(), descriptorPool, nullptr);
	}

	bool VkmDescriptorPool::allocateDescriptor(
		const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const {
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.pSetLayouts = &descriptorSetLayout;
		allocInfo.descriptorSetCount = 1;

		// Might want to create a "DescriptorPoolManager" class that handles this case, and builds
		// a new pool whenever an old pool fills up. But this is beyond our current scope
		// Look at vkguide.dev/docs/extra-chapter/abstracting_descriptors/ for a PRODUCTION Ready abstraction of this
		if (vkAllocateDescriptorSets(vkmDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
			return false;
		}
		return true;
	}

	void VkmDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
		vkFreeDescriptorSets(
			vkmDevice.device(),
			descriptorPool,
			static_cast<uint32_t>(descriptors.size()),
			descriptors.data());
	}

	void VkmDescriptorPool::resetPool() {
		vkResetDescriptorPool(vkmDevice.device(), descriptorPool, 0);
	}

	// *************** Descriptor Writer *********************

	VkmDescriptorWriter::VkmDescriptorWriter(VkmDescriptorSetLayout &setLayout, VkmDescriptorPool &pool)
		: setLayout{ setLayout }, pool{ pool } {}

	VkmDescriptorWriter &VkmDescriptorWriter::writeBuffer(
		uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
		assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

		auto &bindingDescription = setLayout.bindings[binding];

		assert(
			bindingDescription.descriptorCount == 1 &&
			"Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = bufferInfo;
		write.descriptorCount = 1;

		writes.push_back(write);
		return *this;
	}

	VkmDescriptorWriter &VkmDescriptorWriter::writeImage(
		uint32_t binding, VkDescriptorImageInfo *imageInfo) {
		assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

		auto &bindingDescription = setLayout.bindings[binding];

		assert(
			bindingDescription.descriptorCount == 1 &&
			"Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = imageInfo;
		write.descriptorCount = 1;

		writes.push_back(write);
		return *this;
	}

	bool VkmDescriptorWriter::build(VkDescriptorSet &set) {
		bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
		if (!success) {
			return false;
		}
		overwrite(set);
		return true;
	}

	void VkmDescriptorWriter::overwrite(VkDescriptorSet &set) {
		for (auto &write : writes) {
			write.dstSet = set;
		}
		vkUpdateDescriptorSets(pool.vkmDevice.device(), writes.size(), writes.data(), 0, nullptr);
	}

}  // namespace vkm
