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

#include "logi/nvidia/indirect_commands_layout_nvx_impl.hpp"
#include "logi/device/logical_device_impl.hpp"

namespace logi {

IndirectCommandsLayoutNVXImpl::IndirectCommandsLayoutNVXImpl(LogicalDeviceImpl& logicalDevice,
                                                             const vk::IndirectCommandsLayoutCreateInfoNVX& createInfo,
                                                             const std::optional<vk::AllocationCallbacks>& allocator)
  : logicalDevice_(logicalDevice), allocator_(allocator) {
  auto vkDevice = static_cast<vk::Device>(logicalDevice_);
  vkIndirectCommandsLayoutNVX_ =
    vkDevice.createIndirectCommandsLayoutNVX(createInfo, allocator_ ? &allocator_.value() : nullptr, getDispatcher());
}

// region Logi Definitions

VulkanInstanceImpl& IndirectCommandsLayoutNVXImpl::getInstance() const {
  return logicalDevice_.getInstance();
}

PhysicalDeviceImpl& IndirectCommandsLayoutNVXImpl::getPhysicalDevice() const {
  return logicalDevice_.getPhysicalDevice();
}

LogicalDeviceImpl& IndirectCommandsLayoutNVXImpl::getLogicalDevice() const {
  return logicalDevice_;
}

const vk::DispatchLoaderDynamic& IndirectCommandsLayoutNVXImpl::getDispatcher() const {
  return logicalDevice_.getDispatcher();
}

void IndirectCommandsLayoutNVXImpl::destroy() const {
  logicalDevice_.destroyIndirectCommandsLayoutNVX(id());
}

IndirectCommandsLayoutNVXImpl::operator const vk::IndirectCommandsLayoutNVX&() const {
  return vkIndirectCommandsLayoutNVX_;
}

void IndirectCommandsLayoutNVXImpl::free() {
  auto vkDevice = static_cast<vk::Device>(logicalDevice_);
  vkDevice.destroy(vkIndirectCommandsLayoutNVX_, allocator_ ? &allocator_.value() : nullptr, getDispatcher());
  VulkanObject::free();
}

// endregion

} // namespace logi
