#ifndef PROGRAM_LAYOUT_DESCRIPTOR_BINDING_LAYOUT_H
#define PROGRAM_LAYOUT_DESCRIPTOR_BINDING_LAYOUT_H

#include <vulkan/vulkan.hpp>
#include "base/ManagedResource.h"

namespace logi {
namespace internal {

/**
 * @brief	Structure used to accumulate the descriptor binding data during shader reflection.
 */
struct DescriptorBindingInitializer {
	uint32_t binding;				///< Binding index.
	vk::DescriptorType type;		///< Binding descriptor type.	
	uint32_t count;					///< Number of array entries.
	vk::ShaderStageFlags stages;	///< Stages in which the binding should be available.

	/**
	 * @brief	Default constructor.
	 *
	 * @param	binding Binding index.
	 * @param	type	Binding descriptor type.
	 * @param	count	Number of array entries.
	 * @param	stages	Stages in which the binding should be available.
	 */
	DescriptorBindingInitializer(uint32_t binding, vk::DescriptorType type, uint32_t count, vk::ShaderStageFlags stages);
};

} ///! namespace internal

/**
 * @brief	Wraps Vulkan DescriptorBindingLayout data.
 */
struct DescriptorBindingLayout final {
	/**
	 * @brief	Initializes members with the given values.
	 *
	 * @param	binding_initializer Structure containing binding index, descriptor type, number of array entries and stages in which the binding should be available.
	 */
	DescriptorBindingLayout(const internal::DescriptorBindingInitializer& binding_initializer);

	/**
	 * @brief	Build Vulkan binding structure from the data.
	 *
	 * @return	Vulkan binding structure.
	 */
	vk::DescriptorSetLayoutBinding createVkBinding() const;

	const uint32_t binding;				///< Binding index.
	const vk::DescriptorType type;		///< Binding descriptor type.
	const uint32_t	count;				///< Number of array entries.
	const vk::ShaderStageFlags stages;	///< Stages in which the binding should be available.
};

} ///!	namespace logi

#endif ///!	HEADER_GUARD