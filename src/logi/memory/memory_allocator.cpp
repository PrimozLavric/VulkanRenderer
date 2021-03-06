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

#include "logi/memory/memory_allocator.hpp"
#include "logi/device/logical_device.hpp"
#include "logi/device/physical_device.hpp"
#include "logi/instance/vulkan_instance.hpp"
#include "logi/memory/vma_acceleration_structure_nv.hpp"
#include "logi/memory/vma_buffer.hpp"
#include "logi/memory/vma_image.hpp"

namespace logi {

VMABuffer MemoryAllocator::createBuffer(const vk::BufferCreateInfo& bufferCreateInfo,
                                        const VmaAllocationCreateInfo& allocationCreateInfo,
                                        const std::optional<vk::AllocationCallbacks>& allocator) {
  return VMABuffer(object_->createBuffer(bufferCreateInfo, allocationCreateInfo, allocator));
}

void MemoryAllocator::destroyBuffer(const VMABuffer& buffer) {
  object_->destroyBuffer(buffer.id());
}

VMAImage MemoryAllocator::createImage(const vk::ImageCreateInfo& imageCreateInfo,
                                      const VmaAllocationCreateInfo& allocationCreateInfo,
                                      const std::optional<vk::AllocationCallbacks>& allocator) {
  return VMAImage(object_->createImage(imageCreateInfo, allocationCreateInfo, allocator));
}

void MemoryAllocator::destroyImage(const VMAImage& image) {
  object_->destroyImage(image.id());
}

VMAAccelerationStructureNV MemoryAllocator::createAccelerationStructureNV(
  const vk::AccelerationStructureCreateInfoNV& accelerationStructureCreateInfo,
  const VmaAllocationCreateInfo& allocationCreateInfo, const std::optional<vk::AllocationCallbacks>& allocator) {
  return VMAAccelerationStructureNV(
    object_->createAccelerationStructureNV(accelerationStructureCreateInfo, allocationCreateInfo, allocator));
}

void MemoryAllocator::destroyAccelerationStructureNV(const VMAAccelerationStructureNV& accelerationStructure) {
  object_->destroyAccelerationStructureNV(accelerationStructure.id());
}

VulkanInstance MemoryAllocator::getInstance() const {
  return VulkanInstance(object_->getInstance().shared_from_this());
}

PhysicalDevice MemoryAllocator::getPhysicalDevice() const {
  return PhysicalDevice(object_->getPhysicalDevice().shared_from_this());
}

LogicalDevice MemoryAllocator::getLogicalDevice() const {
  return LogicalDevice(object_->getLogicalDevice().shared_from_this());
}

const vk::DispatchLoaderDynamic& MemoryAllocator::getDispatcher() const {
  return object_->getDispatcher();
}

void MemoryAllocator::destroy() const {
  if (object_) {
    object_->destroy();
  }
}

MemoryAllocator::operator const VmaAllocator&() const {
  return object_->operator const VmaAllocator&();
}

}