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

#include "logi/program/pipeline_impl.hpp"
#include "logi/device/logical_device_impl.hpp"

namespace logi {

PipelineImpl::PipelineImpl(LogicalDeviceImpl& logicalDevice, const vk::Pipeline& vkPipeline,
                           const std::optional<vk::AllocationCallbacks>& allocator)
  : logicalDevice_(logicalDevice), vkPipeline_(vkPipeline), allocator_(allocator) {}

// region Vulkan Definitions

vk::DeviceSize PipelineImpl::getRayTracingShaderGroupStackSizeKHR(uint32_t group, 
                                                                  vk::ShaderGroupShaderKHR groupShader) const {
  auto vkDevice = static_cast<vk::Device>(logicalDevice_);
  return vkDevice.getRayTracingShaderGroupStackSizeKHR(vkPipeline_, group, groupShader, getDispatcher());
}

vk::ResultValueType<void>::type PipelineImpl::compileDeferredNV(uint32_t shader) const {
  auto vkDevice = static_cast<vk::Device>(logicalDevice_);
  return vkDevice.compileDeferredNV(vkPipeline_, shader, getDispatcher());
}

typename vk::ResultValueType<std::vector<uint8_t>>::type
  PipelineImpl::getShaderInfoAMD(vk::ShaderStageFlagBits shaderStage, vk::ShaderInfoTypeAMD infoType) const {
  auto vkDevice = static_cast<vk::Device>(logicalDevice_);
  return vkDevice.getShaderInfoAMD(vkPipeline_, shaderStage, infoType, getDispatcher());
}

// endregion

// region Logi Definitions

VulkanInstanceImpl& PipelineImpl::getInstance() const {
  return logicalDevice_.getInstance();
}

PhysicalDeviceImpl& PipelineImpl::getPhysicalDevice() const {
  return logicalDevice_.getPhysicalDevice();
}

LogicalDeviceImpl& PipelineImpl::getLogicalDevice() const {
  return logicalDevice_;
}

const vk::DispatchLoaderDynamic& PipelineImpl::getDispatcher() const {
  return logicalDevice_.getDispatcher();
}

void PipelineImpl::destroy() const {
  logicalDevice_.destroyPipeline(id());
}

PipelineImpl::operator const vk::Pipeline&() const {
  return vkPipeline_;
}

void PipelineImpl::free() {
  auto vkDevice = static_cast<vk::Device>(logicalDevice_);
  vkDevice.destroy(vkPipeline_, allocator_ ? &allocator_.value() : nullptr, getDispatcher());
  VulkanObject::free();
}

// endregion

} // namespace logi