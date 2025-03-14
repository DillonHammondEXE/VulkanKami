#pragma once
#include "vkm_device.h"

namespace vkm {
	class Texture {
	public:
		Texture(VkmDevice &device, const std::string &filepath);
		~Texture();

		Texture(const Texture &) = delete;
		Texture &operator=(const Texture &) = delete;
		Texture(Texture &&) = delete;
		Texture &operator=(Texture &&) = delete;

		VkSampler getSampler() { return sampler; }
		VkImageView getImageView() { return imageView; }
		VkImageLayout getImageLayout() { return imageLayout; }
	private:
		void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
		void generateMipmaps();

		int width, height;
		uint32_t mipLevels;
		VkmDevice& vkmDevice;
		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
		VkSampler sampler;
		VkFormat imageFormat;
		VkImageLayout imageLayout;
	};
}
