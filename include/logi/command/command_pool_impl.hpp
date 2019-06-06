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

#ifndef LOGI_COMMAND_COMMAND_POOL_IMPL_HPP
#define LOGI_COMMAND_COMMAND_POOL_IMPL_HPP

#include <vulkan/vulkan.hpp>
#include "logi/base/vulkan_object.hpp"
#include "logi/device/logical_device_impl.hpp"

namespace logi {

class VulkanInstanceImpl;
class PhysicalDeviceImpl;
class CommandBufferImpl;

class CommandPoolImpl : public VulkanObject<CommandPoolImpl>, public VulkanObjectComposite<CommandBufferImpl> {
 public:
  CommandPoolImpl(LogicalDeviceImpl& logicalDevice, const vk::CommandPoolCreateInfo& createInfo,
                  const std::optional<vk::AllocationCallbacks>& allocator = {});

  // region Sub-Handles

  std::vector<std::shared_ptr<CommandBufferImpl>> allocateCommandBuffers(vk::CommandBufferLevel level,
                                                                         uint32_t commandBufferCount);

  template <typename NextType>
  std::vector<std::shared_ptr<CommandBufferImpl>>
    allocateCommandBuffers(vk::CommandBufferLevel level, uint32_t commandBufferCount, const NextType& next);

  // endregion

  // region Vulkan Declarations

  void freeCommandBuffers(const std::vector<size_t>& cmdBufferIds);

  vk::ResultValueType<void>::type reset(const vk::CommandPoolResetFlags& flags = vk::CommandPoolResetFlags()) const;

  void trim(const vk::CommandPoolTrimFlags& flags = vk::CommandPoolTrimFlags()) const;

  void trimKHR(const vk::CommandPoolTrimFlags& flags = vk::CommandPoolTrimFlags()) const;

  // endregion

  // region Logi Declarations

  VulkanInstanceImpl& getInstance() const;

  PhysicalDeviceImpl& getPhysicalDevice() const;

  LogicalDeviceImpl& getLogicalDevice() const;

  const vk::DispatchLoaderDynamic& getDispatcher() const;

  void destroy() const;

  operator vk::CommandPool() const;

 protected:
  void free() override;

  // endregion

 private:
  LogicalDeviceImpl& logicalDevice_;
  std::optional<vk::AllocationCallbacks> allocator_;
  vk::CommandPool vkCommandPool_;
};

template <typename NextType>
std::vector<std::shared_ptr<CommandBufferImpl>> CommandPoolImpl::allocateCommandBuffers(vk::CommandBufferLevel level,
                                                                                        uint32_t commandBufferCount,
                                                                                        const NextType& next) {
  static_assert(vk::isStructureChainValid<vk::CommandBufferAllocateInfo, NextType>::value,
                "NextType must extend vk::CommandBufferAllocateInfo.");

  auto vkDevice = static_cast<vk::Device>(logicalDevice_);
  vk::CommandBufferAllocateInfo allocateInfo(vkCommandPool_, level, commandBufferCount);
  allocateInfo.pNext = &next;

  std::vector<vk::CommandBuffer> cmdBuffers = vkDevice.allocateCommandBuffers(allocateInfo, getDispatcher());
  std::vector<std::shared_ptr<CommandBufferImpl>> logiCmdBuffers;
  logiCmdBuffers.reserve(cmdBuffers.size());

  for (const auto& buffer : cmdBuffers) {
    logiCmdBuffers.emplace_back(VulkanObjectComposite<CommandBufferImpl>::createObject(*this, buffer));
  }

  return logiCmdBuffers;
}

} // namespace logi

#endif // LOGI_COMMAND_COMMAND_POOL_IMPL_HPP