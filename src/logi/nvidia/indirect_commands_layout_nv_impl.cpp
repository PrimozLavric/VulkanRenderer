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

#include "logi/nvidia/indirect_commands_layout_nv_impl.hpp"
#include "logi/device/logical_device_impl.hpp"

namespace logi {

IndirectCommandsLayoutNVImpl::IndirectCommandsLayoutNVImpl(LogicalDeviceImpl& logicalDevice,
                                                             const vk::IndirectCommandsLayoutCreateInfoNV& createInfo,
                                                             const std::optional<vk::AllocationCallbacks>& allocator)
  : logicalDevice_(logicalDevice), allocator_(allocator) {
  auto vkDevice = static_cast<vk::Device>(logicalDevice_);
  vkIndirectCommandsLayoutNV_ =
    vkDevice.createIndirectCommandsLayoutNV(createInfo, allocator_ ? &allocator_.value() : nullptr, getDispatcher());
}

// region Logi Definitions

VulkanInstanceImpl& IndirectCommandsLayoutNVImpl::getInstance() const {
  return logicalDevice_.getInstance();
}

PhysicalDeviceImpl& IndirectCommandsLayoutNVImpl::getPhysicalDevice() const {
  return logicalDevice_.getPhysicalDevice();
}

LogicalDeviceImpl& IndirectCommandsLayoutNVImpl::getLogicalDevice() const {
  return logicalDevice_;
}

const vk::DispatchLoaderDynamic& IndirectCommandsLayoutNVImpl::getDispatcher() const {
  return logicalDevice_.getDispatcher();
}

void IndirectCommandsLayoutNVImpl::destroy() const {
  logicalDevice_.destroyIndirectCommandsLayoutNV(id());
}

IndirectCommandsLayoutNVImpl::operator const vk::IndirectCommandsLayoutNV&() const {
  return vkIndirectCommandsLayoutNV_;
}

void IndirectCommandsLayoutNVImpl::free() {
  auto vkDevice = static_cast<vk::Device>(logicalDevice_);
  vkDevice.destroy(vkIndirectCommandsLayoutNV_, allocator_ ? &allocator_.value() : nullptr, getDispatcher());
  VulkanObject::free();
}

// endregion

} // namespace logi
