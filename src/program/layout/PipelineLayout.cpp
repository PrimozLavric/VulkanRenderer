#include "logi/program/layout/PipelineLayout.h"
#include <algorithm>
#include <numeric>
#include <spirv_cross.hpp>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "logi/base/LogicalDevice.h"
#include "logi/util/FormatConversion.h"

namespace logi {

PipelineLayout::PipelineLayout(const ProgramManager& prog_manager, const std::vector<PipelineShaderStage>& shaders)
  : DestroyableOwnedHandle<ProgramManager>(prog_manager, true),
    config_(std::make_shared<PipelineLayoutConfig>(shaders)), vk_pipeline_layout_(nullptr) {
  // Sort the shaders in the same order as they come in the pipeline.
  std::sort(config_->shaders.begin(), config_->shaders.end(),
            [](PipelineShaderStage a, PipelineShaderStage b) { return a.stage < b.stage; });

  // Validate stages.
  vk::ShaderStageFlags stages{};
  for (auto& shader : config_->shaders) {
    if (stages & shader.stage) {
      throw std::runtime_error("Multiple shaders with same stage.");
    }

    stages |= shader.stage;
  }

  // Determine pipeline type.
  if (stages & vk::ShaderStageFlagBits::eCompute) {
    if (config_->shaders.size() > 1) {
      throw std::runtime_error("Invalid shader configuration.");
    }

    config_->type = PipelineType::eCompute;
  } else if ((stages & vk::ShaderStageFlagBits::eVertex) && (stages & vk::ShaderStageFlagBits::eFragment)) {
    config_->type = PipelineType::eGraphical;
  } else {
    throw std::runtime_error("Invalid shader configuration.");
  }

  // Perform shader reflection.
  shaderReflection(getOwner().getLogicalDeviceHandle());

  // Update descriptor count.
  for (auto it = config_->descriptor_sets.begin(); it != config_->descriptor_sets.end(); ++it) {
    config_->combined_descriptors_count += it->getDescriptorsCount();
  }

  // Build Vulkan pipeline layout.
  buildVkPipelineLayout(device);
}

PipelineType PipelineLayout::getPipelineType() const {
  return config_->type;
}

std::vector<vk::PipelineShaderStageCreateInfo> PipelineLayout::getVkShaderHandles() const {
  std::vector<vk::PipelineShaderStageCreateInfo> vk_shaders;
  vk_shaders.reserve(config_->shaders.size());

  // Fetch ShaderStageCreateInfo-s.
  for (auto& shader : config_->shaders) {
    vk_shaders.emplace_back(shader.build());
  }

  return vk_shaders;
}

const vk::PipelineLayout& PipelineLayout::getVkHandle() const {
  if (!alive()) {
    throw std::runtime_error("Called getVkHandle on destroyed PipelineLayout.");
  }

  return vk_pipeline_layout_->get();
}

const DescriptorsCount& PipelineLayout::getDescriptorsCount() const {
  return config_->combined_descriptors_count;
}

const DescriptorSetLayout& PipelineLayout::getDescriptorSetLayout(size_t set) const {
  return config_->descriptor_sets[set];
}

const std::vector<VertexAttributeLayout>& PipelineLayout::getAttributeLayouts() const {
  return config_->attributes;
}

size_t PipelineLayout::getDescriptorSetCount() const {
  return config_->descriptor_sets.size();
}

void PipelineLayout::shaderReflection(const vk::Device& device) const {
  std::vector<std::vector<internal::DescriptorBindingInitializer>> desc_sets_initializers{};
  std::vector<internal::PushConstantRangeInitializer> push_consts_initializers{};

  for (PipelineShaderStage& shader : config_->shaders) {
    spirv_cross::Compiler comp(shader.code());

    // The SPIR-V is now parsed, and we can perform reflection on it.
    spirv_cross::ShaderResources resources = comp.get_shader_resources();
    vk::ShaderStageFlagBits stage = shader.stage;

#pragma region VertexAttributes
    // Parse attributes if it's a vertex shader.
    if (stage == vk::ShaderStageFlagBits::eVertex) {
      // Parse attribute configuration.
      for (auto& resource : resources.stage_inputs) {
        const uint32_t binding = comp.get_decoration(resource.id, spv::DecorationBinding);
        const uint32_t location = comp.get_decoration(resource.id, spv::DecorationLocation);
        const auto resource_type = comp.get_type_from_variable(resource.id);
        const uint32_t offset = 0;
        const vk::Format format = getVertexBufferFormat(resource_type);
        const uint32_t stride = (resource_type.width * resource_type.vecsize * resource_type.columns) /
                                8; // Size of the single element in bytes.
        const vk::VertexInputRate input_rate = vk::VertexInputRate::eVertex;

        // Add attribute description
        config_->attributes.emplace_back(VertexAttributeLayout(location, binding, format, offset, stride, input_rate));
      }
    }
#pragma endregion

#pragma region DescriptorBindings
    // Helper lambda used to add new DescriptorSetLayoutMeta entry to the shader meta.
    auto addDescriptorMeta = [&desc_sets_initializers, &comp, &stage](const spirv_cross::Resource& resource,
                                                                      vk::DescriptorType type) {
      auto resource_type = comp.get_type_from_variable(resource.id);

      // Parse data.
      size_t set = static_cast<size_t>(comp.get_decoration(resource.id, spv::DecorationDescriptorSet));
      uint32_t binding = comp.get_decoration(resource.id, spv::DecorationBinding);
      uint32_t count =
        std::accumulate(resource_type.array.begin(), resource_type.array.end(), 1, std::multiplies<uint32_t>());

      // If the set does not yet exist add a new one.
      while (desc_sets_initializers.size() <= set) {
        desc_sets_initializers.emplace_back(std::vector<internal::DescriptorBindingInitializer>());
      }

      for (auto it = desc_sets_initializers[set].begin(); it != desc_sets_initializers[set].end(); it++) {
        // Check if the entry for this binding already exists.
        if (it->binding == binding) {
          if (it->type != type || it->count != count) {
            throw std::runtime_error("Shader binding miss match.");
          }

          it->stages |= stage;
          return;
        } else if (it->binding > binding) {
          desc_sets_initializers[set].insert(it, internal::DescriptorBindingInitializer(binding, type, count, stage));
          return;
        }
      }

      // Emplace at the end of the set.
      desc_sets_initializers[set].emplace_back(internal::DescriptorBindingInitializer(binding, type, count, stage));
    };

    // Look at: http://vulkan-spec-chunked.ahcox.com/ch13s01.html

    // Add uniform buffers meta.
    for (auto& resource : resources.uniform_buffers) {
      addDescriptorMeta(resource, vk::DescriptorType::eUniformBuffer);
    }

    // Add storage buffers meta.
    for (auto& resource : resources.storage_buffers) {
      addDescriptorMeta(resource, vk::DescriptorType::eStorageBuffer);
    }

    // Add storage images meta.
    for (auto& resource : resources.storage_images) {
      if (comp.get_type_from_variable(resource.id).image.dim == spv::DimBuffer) {
        // If it's a buffer it is a storage texel buffer not storage image.
        addDescriptorMeta(resource, vk::DescriptorType::eStorageTexelBuffer);
      } else {
        addDescriptorMeta(resource, vk::DescriptorType::eStorageImage);
      }
    }

    // Add sampled images (separate) meta.
    for (auto& resource : resources.separate_images) {
      addDescriptorMeta(resource, vk::DescriptorType::eSampledImage);
    }

    // Add sampled images (combined with sampler) meta.
    for (auto& resource : resources.sampled_images) {
      // If it's a buffer it is a uniform texel buffer not combined image sampler.
      if (comp.get_type_from_variable(resource.id).image.dim == spv::DimBuffer) {
        addDescriptorMeta(resource, vk::DescriptorType::eUniformTexelBuffer);
      } else {
        addDescriptorMeta(resource, vk::DescriptorType::eCombinedImageSampler);
      }
    }

    // Add sampler meta (used in conjunction with a sampled image (separate).
    for (auto& resource : resources.separate_samplers) {
      addDescriptorMeta(resource, vk::DescriptorType::eSampler);
    }

    // Add subpass attachments
    for (auto& resource : resources.subpass_inputs) {
      addDescriptorMeta(resource, vk::DescriptorType::eInputAttachment);
    }

    // Add Push constants
    for (auto& resource : resources.push_constant_buffers) {
      for (spirv_cross::BufferRange const_range : comp.get_active_buffer_ranges(resource.id)) {
        auto it = push_consts_initializers.begin();

        for (; it != push_consts_initializers.end(); it++) {
          if (it->offset == const_range.offset && it->size == const_range.range) {
            // Add stage.
            it->stages |= stage;
            break;
          } else if (it->offset > const_range.offset) {
            it = push_consts_initializers.insert(
              it, internal::PushConstantRangeInitializer(static_cast<uint32_t>(const_range.offset),
                                                         static_cast<uint32_t>(const_range.range), stage));
            break;
          }
        }

        // Emplace at the end of the const.
        if (it == push_consts_initializers.end()) {
          push_consts_initializers.emplace_back(internal::PushConstantRangeInitializer(
            static_cast<uint32_t>(const_range.offset), static_cast<uint32_t>(const_range.range), stage));
        }
      }
    }
#pragma endregion
  }

  // Initialize descriptor sets.
  for (const std::vector<internal::DescriptorBindingInitializer>& set : desc_sets_initializers) {
    DescriptorSetLayout descriptor_set = descriptor_set_hm_->createHandle<DescriptorSetLayout>(device, set);
    config_->descriptor_sets.emplace_back(descriptor_set);
  }

  // Initialize push constants.
  for (size_t i = 0; i < push_consts_initializers.size(); i++) {
    // Check for push constant overlap
    if ((i + 1) < push_consts_initializers.size() &&
        push_consts_initializers[i].offset + push_consts_initializers[i].size >
          push_consts_initializers[i + 1].offset) {
      throw std::runtime_error("Push constant overlap.");
    }

    config_->push_constants.emplace_back(PushConstantRange(push_consts_initializers[i]));
  }
}

void PipelineLayout::buildVkPipelineLayout(const vk::Device& device) {
  // Retrieve descriptor set vulkan handles
  std::vector<vk::DescriptorSetLayout> vk_desc_set_layouts{};
  vk_desc_set_layouts.reserve(config_->descriptor_sets.size());

  for (const DescriptorSetLayout& desc_set : config_->descriptor_sets) {
    vk_desc_set_layouts.emplace_back(desc_set.getVkHandle());
  }

  // Build vulkan push constants.
  std::vector<vk::PushConstantRange> vk_push_constants{};
  vk_push_constants.reserve(config_->push_constants.size());

  for (PushConstantRange& push_constant : config_->push_constants) {
    vk_push_constants.emplace_back(push_constant.getVkHandle());
  }

  vk::PipelineLayoutCreateInfo layout_ci{};
  layout_ci.pSetLayouts = vk_desc_set_layouts.data();
  layout_ci.setLayoutCount = static_cast<uint32_t>(vk_desc_set_layouts.size());
  layout_ci.pPushConstantRanges = vk_push_constants.data();
  layout_ci.pushConstantRangeCount = static_cast<uint32_t>(vk_push_constants.size());

  vk_pipeline_layout_ = std::make_shared<ManagedVkPipelineLayout>(device, device.createPipelineLayout(layout_ci));
}

#pragma endregion

} // namespace logi