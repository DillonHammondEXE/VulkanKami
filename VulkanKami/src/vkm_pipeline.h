#pragma once

#include "vkm_device.h"
#include <string>
#include <vector>

namespace vkm {

	struct PipelineConfigInfo {};

	class VkmPipeline {
	public:
		VkmPipeline(
			VkmDevice& device,
			const std::string& vertFilepath, 
			const std::string& fragFilepath,
			const PipelineConfigInfo& configInfo);
		~VkmPipeline() {}

		// "Resource Acquisition Is Initialization" (RAII)
		VkmPipeline(const VkmPipeline&) = delete; 
		void operator=(const VkmPipeline&) = delete;

		static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);


	private:
		static std::vector<char> readFile(const std::string& filepath);

		void createGraphicsPipeline(
			const std::string& vertFilepath, 
			const std::string& fragFilepath,
			const PipelineConfigInfo& configInfo);

		// shaderModule is a double pointer
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule); 

		VkmDevice& vkmDevice; // Rare case where we use a member variable for a reference (aggregation)
		// Type Def Pointers
		VkPipeline graphicsPipeline; 
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
	};

} // Namespace vkm
