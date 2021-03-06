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

#ifndef LOGI_SWAPCHAIN_SWAPCHAIN_KHR_IMPL_HPP
#define LOGI_SWAPCHAIN_SWAPCHAIN_KHR_IMPL_HPP

#include "logi/base/common.hpp"
#include <optional>
#include "logi/base/vulkan_object.hpp"
#include "logi/structures/extension.hpp"

namespace logi {

class LogicalDeviceImpl;
class VulkanInstanceImpl;
class PhysicalDeviceImpl;
class SwapchainImageImpl;

class SwapchainKHRImpl : public VulkanObject,
                         public std::enable_shared_from_this<SwapchainKHRImpl>,
                         public VulkanObjectComposite<SwapchainImageImpl> {
 public:
  SwapchainKHRImpl(LogicalDeviceImpl& logicalDevice, const vk::SwapchainCreateInfoKHR& createInfo,
                   const std::optional<vk::AllocationCallbacks>& allocator = {});

  SwapchainKHRImpl(LogicalDeviceImpl& logicalDevice, const vk::SwapchainKHR& swapchain,
                   const std::optional<vk::AllocationCallbacks>& allocator = {});

  // region Vulkan Declarations

  const std::vector<std::shared_ptr<SwapchainImageImpl>>& getImagesKHR();

  vk::ResultValue<uint32_t> acquireNextImageKHR(uint64_t timeout, const vk::Semaphore& semaphore,
                                                const vk::Fence& fence) const;

  vk::ResultValue<uint32_t> acquireNextImage2KHR(uint64_t timeout, const vk::Semaphore& semaphore,
                                                 const vk::Fence& fence, uint32_t deviceMask,
                                                 const ConstVkNextProxy<vk::AcquireNextImageInfoKHR>& next) const;

  vk::ResultValueType<uint64_t>::type getCounterEXT(vk::SurfaceCounterFlagBitsEXT counter) const;

  vk::Result getStatusKHR() const;

  void setLocalDimmingAMD(vk::Bool32 localDimmingEnable) const;

  typename vk::ResultValueType<std::vector<vk::PastPresentationTimingGOOGLE>>::type
    getPastPresentationTimingGOOGLE() const;

  typename vk::ResultValueType<vk::RefreshCycleDurationGOOGLE>::type getRefreshCycleDurationGOOGLE() const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
  vk::ResultValueType<void>::type acquireFullScreenExclusiveModeEXT() const;

  vk::ResultValueType<void>::type releaseFullScreenExclusiveModeEXT() const;
#endif

  // endregion

  // region Logi Declarations

  VulkanInstanceImpl& getInstance() const;

  PhysicalDeviceImpl& getPhysicalDevice() const;

  LogicalDeviceImpl& getLogicalDevice() const;

  const vk::DispatchLoaderDynamic& getDispatcher() const;

  void destroy() const;

  operator const vk::SwapchainKHR&() const;

 protected:
  void free() override;

  void acquireImageInternal();

  // endregion

 private:
  LogicalDeviceImpl& logicalDevice_;
  std::optional<vk::AllocationCallbacks> allocator_;
  vk::SwapchainKHR vkSwapchainKHR_;
  std::vector<std::shared_ptr<SwapchainImageImpl>> images_;
};

} // namespace logi

#endif // LOGI_SWAPCHAIN_SWAPCHAIN_KHR_IMPL_HPP
