/*
 * Descriptor pool
 *
 * Copyright (C) 2017 by Primoz Lavric
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef DESCRIPTORS_DESCRIPTOR_POOL_H
#define DESCRIPTORS_DESCRIPTOR_POOL_H

#include <vulkan/vulkan.hpp>
#include "logi/base/ExtensionObject.h"
#include "logi/base/Handle.h"
#include "logi/base/ManagedResource.h"
#include "logi/descriptors/DescriptorSet.h"
#include "logi/program/layout/PipelineLayout.h"

namespace logi {

class DescriptorPool : public DestroyableOwnedHandle<LogicalDevice>,
                       public HandleGenerator<DescriptorPool, DescriptorSet> {
 public:
  /**
   * @brief	Construct placeholder DescriptorPool handle.
   */
  DescriptorPool() = default;

  /**
   * @brief   Create and configure Vulkan descriptor pool.
   *
   * @param	device      Logical device handle
   * @param	pool_sizes  Descriptor pool sizes.
   * @param	flags       Additional DescriptorPool configuration flags.
   */
  DescriptorPool(const LogicalDevice& device, const DescriptorsCount& pool_sizes,
                 const vk::DescriptorPoolCreateFlags& flags);

  /**
   * @brief	Allocate descriptors sets using the given descriptor set layouts.
   *
   * @tparam	ExtensionTypes  Types of extension objects.
   * @param	set_layouts     Descriptor set layout handles.
   * @param	extensions      Variadic allocation info extensions
   * @return	Vector of allocated descriptor sets ordered in the same order as the given layouts.
   */
  template <typename... ExtensionTypes>
  std::vector<DescriptorSet> allocateDescriptorSets(const std::vector<DescriptorSetLayout>& set_layouts,
                                                    ExtensionTypes... extensions) const;

  /**
   * @brief	Allocate descriptor sets using the given descriptor set layout.
   *
   * @tparam	ExtensionTypes  Types of extension objects.
   * @param	set_layout     Descriptor set layout handle.
   * @param	extensions      Variadic allocation info extensions
   * @return	Allocated descriptor set.
   */
  template <typename... ExtensionTypes>
  DescriptorSet allocateDescriptorSet(const DescriptorSetLayout& set_layout, ExtensionTypes... extensions) const;

  /**
   * @brief	Free the given descriptor sets.
   *
   * @param	descriptor_sets Vectir of descriptor sets that will be freed.
   */
  void freeDescriptorSets(const std::vector<DescriptorSet>& descriptor_sets) const;

  /**
   * @brief	Resets descriptor pool. Frees all descriptor sets allocated from this pool.
   *
   * @param	reset_flags Additional reset options.
   */
  void reset(const vk::DescriptorPoolResetFlags& reset_flags) const;

 protected:
  /**
   * @brief   Destroys all dependent handle and frees the pool.
   */
  void free() override;

 private:
  using ManagedVkDescriptorPool =
    ManagedResource<vk::Device, vk::DescriptorPool, vk::DispatchLoaderStatic, &vk::Device::destroyDescriptorPool>;

  /**
   * @brief	Contains configuration for DescriptorPool.
   */
  struct Data {
    /**
     * @brief	Default constructor. Initializes values to defaults.
     *
     * @param vk_descriptor_pool  Vulkan descriptor pool handle.
     * @param	pool_sizes          Specifies required pool size for each descriptor type.
     * @param	flags               A bitmask of vk::DescriptorPoolCreateFlagBits describing additional parameters of the
     *                            DescriptorPool.
     */
    Data(ManagedVkDescriptorPool vk_descriptor_pool, const DescriptorsCount& pool_sizes,
         const vk::DescriptorPoolCreateFlags& flags);

    /**
     * Vulkan DescriptorPool handle.
     */
    ManagedVkDescriptorPool vk_descriptor_pool;

    /**
     * Specifies required pool size for each descriptor type.
     */
    DescriptorsCount pool_sizes;

    /**
     * A bitmask of vk::DescriptorPoolCreateFlagBits describing additional parameters of the DescriptorPool.
     */
    vk::DescriptorPoolCreateFlags flags;
  };

  /**
   * Contains descriptor pool data.
   */
  std::shared_ptr<Data> data_;
};

template <typename... ExtensionTypes>
std::vector<DescriptorSet> DescriptorPool::allocateDescriptorSets(const std::vector<DescriptorSetLayout>& set_layouts,
                                                                  ExtensionTypes... extensions) const {
  // Generate descriptor set layouts.
  std::vector<vk::DescriptorSetLayout> vk_layouts;
  vk_layouts.reserve(set_layouts.size());

  for (const DescriptorSetLayout& layout : set_layouts) {
    vk_layouts.push_back(layout.getVkHandle());
  }

  // Generate allocate info.
  vk::DescriptorSetAllocateInfo set_alloc_info{};
  set_alloc_info.descriptorPool = data_->vk_descriptor_pool.get();
  set_alloc_info.descriptorSetCount = vk_layouts.size();
  set_alloc_info.pSetLayouts = vk_layouts.data();
  set_alloc_info.pNext = BuildableExtension::buildExtensions(extensions...);

  // Allocate descriptor sets.
  const vk::Device& device = getOwner<LogicalDevice>();
  std::vector<vk::DescriptorSet> vk_descriptor_sets = device.allocateDescriptorSets(set_alloc_info);
  std::vector<DescriptorSet> descriptor_sets;
  descriptor_sets.reserve(vk_descriptor_sets.size());

  // Create handles.
  for (size_t i = 0; i < vk_descriptor_sets.size(); i++) {
    descriptor_sets.emplace_back(
      HandleGenerator<DescriptorPool, DescriptorSet>::template createHandle(vk_descriptor_sets[i], set_layouts[i]));
  }

  return descriptor_sets;
}

template <typename... ExtensionTypes>
DescriptorSet DescriptorPool::allocateDescriptorSet(const DescriptorSetLayout& set_layout,
                                                    ExtensionTypes... extensions) const {
  // Generate allocate info.
  vk::DescriptorSetAllocateInfo set_alloc_info{};
  set_alloc_info.descriptorPool = data_->vk_descriptor_pool.get();
  set_alloc_info.descriptorSetCount = 1u;
  set_alloc_info.pSetLayouts = &set_layout.getVkHandle();
  set_alloc_info.pNext = BuildableExtension::buildExtensions(extensions...);

  // Allocate descriptor sets.
  vk::Device device = getOwner < LogicalDevice();

  return HandleGenerator<DescriptorPool, DescriptorSet>::template createHandle(
    device.allocateDescriptorSets(set_alloc_info)[0], set_layout);
}

} // namespace logi

#endif
