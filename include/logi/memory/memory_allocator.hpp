/**
 * Project Logi source code
 * Copyright (C) 2019 Primoz Lavric
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOGI_MEMORY_MEMORY_ALLOCATOR_HPP
#define LOGI_MEMORY_MEMORY_ALLOCATOR_HPP

#include <optional>
#include "logi/base/common.hpp"
#include "logi/base/handle.hpp"
#include "logi/memory/memory_allocator_impl.hpp"

namespace logi {

class VulkanInstance;
class PhysicalDevice;
class LogicalDevice;
class VMABuffer;
class VMAImage;
class VMAAccelerationStructureNV;

/**
 * @brief Implements allocating and destruction of resources using <a href="https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/index.html">VMA(Vulkan Memory Allocator)</a> 
 */
class MemoryAllocator : public Handle<MemoryAllocatorImpl> {
 public:
  using Handle::Handle;

  VMABuffer createBuffer(const vk::BufferCreateInfo& bufferCreateInfo,
                         const VmaAllocationCreateInfo& allocationCreateInfo,
                         const std::optional<vk::AllocationCallbacks>& allocator = {});

  void destroyBuffer(const VMABuffer& buffer);

  VMAImage createImage(const vk::ImageCreateInfo& imageCreateInfo, const VmaAllocationCreateInfo& allocationCreateInfo,
                       const std::optional<vk::AllocationCallbacks>& allocator = {});

  void destroyImage(const VMAImage& image);

  VMAAccelerationStructureNV
    createAccelerationStructureNV(const vk::AccelerationStructureCreateInfoNV& accelerationStructureCreateInfo,
                                  const VmaAllocationCreateInfo& allocationCreateInfo,
                                  const std::optional<vk::AllocationCallbacks>& allocator = {});

  void destroyAccelerationStructureNV(const VMAAccelerationStructureNV& accelerationStructure);

  // region Logi Declarations

  VulkanInstance getInstance() const;

  PhysicalDevice getPhysicalDevice() const;

  LogicalDevice getLogicalDevice() const;

  const vk::DispatchLoaderDynamic& getDispatcher() const;

  void destroy() const;

  operator const VmaAllocator&() const;

  // endregion
};

} // namespace logi

#endif // LOGI_MEMORY_MEMORY_ALLOCATOR_HPP
