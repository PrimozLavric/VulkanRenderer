/*
* Shader manager.
*
* Copyright (C) 2017 by Primoz Lavric
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <vector>
#include <map>
#include <vulkan/vulkan.hpp>
#include <filesystem>
#include "VulkanDevice.h"

namespace vkr {
namespace filesystem = std::experimental::filesystem;  ///< todo(plavric): Change when the filesystem will no longer be experimental.

class ShaderManager {
public:
	/**
	 * @brief Load the shaders meta data stored in "shader_map.json" file. This file is expected in the root of the given directory.
	 * @throw std::runtime_error if something goes wrong while loading shaders meta data.
	 *
	 * @param shaders_path Path to the directory containing "shader_map.json" file.
	 */
	void loadShaders(const std::string& shaders_path);

	/**
	 * @brief Retrieve shader pipeline stages. When first called it generates and caches the pipeline stages. All subsequent calls
	 * return cached pipeline stages.
	 *
	 * @param device Device for which the pipeline stages will be generated.
	 * @return Vector containing shader pipeline stages.
	 */
	std::vector<vk::PipelineShaderStageCreateInfo> getShaderPipelineStages(VulkanDevice& device, const std::string& pipeline_id);
private:
	/**
	 * @brief Internal structure holding the shader module meta data.
	 */
	struct ShaderModuleInfo {
		vk::ShaderStageFlagBits stage; ///< Pipeline stage of the shader.
		filesystem::path shader_path; ///< Path to the SPIR shader file.
	};

	/**
	 * @brief Read raw data from the SPIRV shader file located on the given path.
	 * @throw std::runtime_error if something hoes wrong while reading the shader file.
	 * 
	 * @param path Path to the SPIRV shader file.
	 * @return Raw SPIRV shader data.
	 */
	std::vector<char> readShaderFile(const filesystem::path& path);

	/**
	 * @brief Create shader module for the given device using the given code.
	 *
	 * @param device Target device.
	 * @param code Shader code used to create the module.
	 * @return Created shader module.
	 */
	vk::ShaderModule createShaderModule(vk::Device& device, const std::vector<char>& code);

	std::map<std::string, std::vector<ShaderModuleInfo>> shader_pipelines_; ///< Maps pipeline id to the shader pipeline modules meta data.
};

}

#endif