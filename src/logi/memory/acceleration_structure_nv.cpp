/**
 * Project Logi source code
 * Copyright (C) 2019 Lana Besevic
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

#include "logi/memory/acceleration_structure_nv.hpp"
#include "logi/device/logical_device_impl.hpp"
#include "logi/device/physical_device_impl.hpp"
#include "logi/instance/vulkan_instance.hpp"
#include "logi/instance/vulkan_instance_impl.hpp"
#include "logi/memory/acceleration_structure_nv_impl.hpp"

namespace logi {

vk::MemoryRequirements2KHR AccelerationStructureNV::getMemoryRequirementsNV(
  vk::AccelerationStructureMemoryRequirementsTypeNV type,
  const ConstVkNextProxy<vk::AccelerationStructureMemoryRequirementsInfoNV>& next) const {
  return object_->getMemoryRequirementsNV(type, next);
}

vk::ResultValueType<void>::type
  AccelerationStructureNV::bindMemory(vk::DeviceMemory memory, vk::DeviceSize memoryOffset,
                                      const vk::ArrayProxy<uint32_t>& deviceIndices,
                                      const ConstVkNextProxy<vk::BindAccelerationStructureMemoryInfoNV>& next) const {
  object_->bindMemory(memory, memoryOffset, deviceIndices, next);
}

// region Logi Definitions

VulkanInstance AccelerationStructureNV::getInstance() const {
  return VulkanInstance(object_->getInstance().shared_from_this());
}

PhysicalDevice AccelerationStructureNV::getPhysicalDevice() const {
  return PhysicalDevice(object_->getPhysicalDevice().shared_from_this());
}

LogicalDevice AccelerationStructureNV::getLogicalDevice() const {
  return LogicalDevice(object_->getLogicalDevice().shared_from_this());
}

const vk::DispatchLoaderDynamic& AccelerationStructureNV::getDispatcher() const {
  return object_->getDispatcher();
}

void AccelerationStructureNV::destroy() const {
  if (object_) {
    object_->destroy();
  }
}

AccelerationStructureNV::operator const vk::AccelerationStructureNV&() const {
  static vk::AccelerationStructureNV nullHandle(nullptr);
  return (object_) ? object_->operator const vk::AccelerationStructureNV&() : nullHandle;
}

// endregion

} // namespace logi
