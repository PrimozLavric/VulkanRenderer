#include "hello_triangle.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

void HelloTriangle::loadShaders() {
  shaderReflection_ = utility::loadShaders(vulkanState_,
                                           "./shaders/triangle.vert.spv",
                                           "./shaders/triangle.frag.spv");

  pipelineLayoutData_ = utility::createPipelineLayout(vulkanState_, shaderReflection_);
}

void HelloTriangle::allocateBuffers() {
  // Set allocator
  logi::MemoryAllocator allocator = vulkanState_.defaultLogicalDevice_->createMemoryAllocator();
  vulkanState_.addAllocator("MainAlloc", allocator);
  vulkanState_.setDefaultAllocator("MainAlloc");

  // Create vertex buffer
  utility::BufferAllocateInfo vertexBufferAllocateInfo = {};
  vertexBufferAllocateInfo.data = vertices.data();
  vertexBufferAllocateInfo.size = vertices.size() * sizeof(Vertex);
  vertexBufferAllocateInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
  vertexBufferAllocateInfo.sharingMode = vk::SharingMode::eExclusive;

  std::vector<utility::BufferAllocateInfo> bufferAllocateInfo = {vertexBufferAllocateInfo};
  std::vector<logi::VMABuffer> buffers;

  utility::allocateBuffer(vulkanState_, VMA_MEMORY_USAGE_CPU_TO_GPU, bufferAllocateInfo, buffers);
  vertexBuffer_ = buffers[0];

  // Create and init matrices UBO buffer
  VmaAllocationCreateInfo allocationInfo = {};
  allocationInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;

  vk::BufferCreateInfo matricesBufferInfo;
  matricesBufferInfo.size = sizeof(uboMatrices);
  matricesBufferInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
  matricesBufferInfo.sharingMode = vk::SharingMode::eExclusive;

  for (size_t i = 0; i < swapchainImages_.size(); i++) {
    matricesUBOBuffers_.emplace_back(vulkanState_.defaultAllocator_->createBuffer(matricesBufferInfo, allocationInfo));
  }

  updateMatrixBuffers();
}

void HelloTriangle::updateMatrixBuffers() {
  // Update matrices
  uboMatrices.projectionMatrix = glm::perspective(
    glm::radians(60.0f), (float) swapchainImageExtent_.width / (float) swapchainImageExtent_.height, 0.1f, 256.0f);

  uboMatrices.viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(cameraPos.x, cameraPos.y, zoom));

  uboMatrices.modelMatrix = glm::mat4(1.0f);
  uboMatrices.modelMatrix = glm::rotate(uboMatrices.modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  uboMatrices.modelMatrix = glm::rotate(uboMatrices.modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  uboMatrices.modelMatrix = glm::rotate(uboMatrices.modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

  for (const auto& buffer : matricesUBOBuffers_) {
    buffer.writeToBuffer(&uboMatrices, sizeof(uboMatrices));
  }
}

void HelloTriangle::initializeDescriptorSets() {
  // Create descriptor pool.
  vk::DescriptorPoolSize poolSize;
  poolSize.type = vk::DescriptorType::eUniformBuffer;
  poolSize.descriptorCount = static_cast<uint32_t>(swapchainImages_.size());

  vk::DescriptorPoolCreateInfo poolInfo;
  poolInfo.pPoolSizes = &poolSize;
  poolInfo.poolSizeCount = 1u;
  poolInfo.maxSets = static_cast<uint32_t>(swapchainImages_.size());

  descriptorPool_ = vulkanState_.defaultLogicalDevice_->createDescriptorPool(poolInfo);

  // Create descriptor sets.
  std::vector<vk::DescriptorSetLayout> layouts(swapchainImages_.size(), pipelineLayoutData_.descriptorSetLayouts[0]);
  descriptorSets_ = descriptorPool_.allocateDescriptorSets(layouts);

  for (size_t i = 0; i < descriptorSets_.size(); i++) {
    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = matricesUBOBuffers_[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(uboMatrices);

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = descriptorSets_[i];
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vulkanState_.defaultLogicalDevice_->updateDescriptorSets(descriptorWrite);
  }
}

void HelloTriangle::createRenderPass() {
  vk::AttachmentDescription colorAttachment;
  colorAttachment.format = swapchainImageFormat_;
  colorAttachment.samples = vk::SampleCountFlagBits::e1;
  colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
  colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
  colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eStore;
  colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
  colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentReference colorAttachmentRef;
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription subpass;
  subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  vk::SubpassDependency dependency;
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  dependency.srcAccessMask = {};
  dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

  vk::RenderPassCreateInfo renderPassCreateInfo;
  renderPassCreateInfo.attachmentCount = 1;
  renderPassCreateInfo.pAttachments = &colorAttachment;
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpass;
  renderPassCreateInfo.dependencyCount = 1;
  renderPassCreateInfo.pDependencies = &dependency;

  renderPass_ = vulkanState_.defaultLogicalDevice_->createRenderPass(renderPassCreateInfo);
}

void HelloTriangle::createGraphicalPipeline() {
  // Destroy existing pipeline.
  if (pipeline_) {
    pipeline_.destroy();
  }

  // Pipeline
  vk::PipelineShaderStageCreateInfo vertShaderStageInfo;
  vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
  vertShaderStageInfo.module = shaderReflection_.vertexShader;
  vertShaderStageInfo.pName = "main";

  vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
  fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
  fragShaderStageInfo.module = shaderReflection_.fragmentShader;
  fragShaderStageInfo.pName = "main";

  vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  std::vector<logi::VertexAttributeReflectionInfo> vtxAttributeInfo =
    shaderReflection_.vertexShader.getVertexAttributeReflectionInfo("main");

  std::vector<vk::VertexInputAttributeDescription> vtxAttributeDescriptions;
  vtxAttributeDescriptions.reserve(vtxAttributeInfo.size());

  uint32_t offset = 0u;
  for (const auto& entry : vtxAttributeInfo) {
    vtxAttributeDescriptions.emplace_back(entry.location, kVertexBinding, entry.format, offset);
    offset += entry.elementSize;
  }

  vk::VertexInputBindingDescription vtxBindingDescription(kVertexBinding, sizeof(Vertex), vk::VertexInputRate::eVertex);

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
  vertexInputInfo.pVertexBindingDescriptions = &vtxBindingDescription;
  vertexInputInfo.vertexBindingDescriptionCount = 1u;
  vertexInputInfo.pVertexAttributeDescriptions = vtxAttributeDescriptions.data();
  vertexInputInfo.vertexAttributeDescriptionCount = vtxAttributeDescriptions.size();

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
  inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  vk::Viewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float) swapchainImageExtent_.width;
  viewport.height = (float) swapchainImageExtent_.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vk::Rect2D scissor;
  scissor.extent = swapchainImageExtent_;

  vk::PipelineViewportStateCreateInfo viewportState;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  vk::PipelineRasterizationStateCreateInfo rasterizer;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = vk::PolygonMode::eFill;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = vk::CullModeFlagBits::eNone;
  rasterizer.frontFace = vk::FrontFace::eClockwise;
  rasterizer.depthBiasEnable = VK_FALSE;

  vk::PipelineMultisampleStateCreateInfo multisampling;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
  vk::SampleMask sampleMask(0xFFFFFF);
  multisampling.pSampleMask = &sampleMask;

  vk::PipelineColorBlendAttachmentState colorBlendAttachment;
  colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  colorBlendAttachment.blendEnable = VK_FALSE;

  vk::PipelineColorBlendStateCreateInfo colorBlending;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = vk::LogicOp::eCopy;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  vk::GraphicsPipelineCreateInfo pipelineInfo;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = pipelineLayoutData_.layout;
  pipelineInfo.renderPass = renderPass_;
  pipelineInfo.subpass = 0;

  pipeline_ = vulkanState_.defaultLogicalDevice_->createGraphicsPipeline(pipelineInfo);
}

void HelloTriangle::createFrameBuffers() {
  // Destroy previous framebuffers.
  for (const auto& framebuffer : framebuffers_) {
    framebuffer.destroy();
  }
  framebuffers_.clear();

  // Create new framebuffers.
  for (const auto& imageView : swapchainImageViews_) {
    vk::FramebufferCreateInfo createInfo;
    createInfo.renderPass = renderPass_;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &static_cast<const vk::ImageView&>(imageView);
    createInfo.width = swapchainImageExtent_.width;
    createInfo.height = swapchainImageExtent_.height;
    createInfo.layers = 1;

    framebuffers_.emplace_back(vulkanState_.defaultLogicalDevice_->createFramebuffer(createInfo));
  }
}

void HelloTriangle::recordCommandBuffers() {
  // Destroy old command buffers.
  for (const auto& cmdBuffer : primaryGraphicsCmdBuffers_) {
    cmdBuffer.reset();
  }

  for (size_t i = 0; i < primaryGraphicsCmdBuffers_.size(); i++) {
    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;

    primaryGraphicsCmdBuffers_[i].begin(beginInfo);

    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = renderPass_;
    renderPassInfo.framebuffer = framebuffers_[i];
    renderPassInfo.renderArea.extent = swapchainImageExtent_;

    vk::ClearValue clearValue;
    clearValue.color.setFloat32({0.0, 0.0, 0.0, 1.0});
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    primaryGraphicsCmdBuffers_[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    primaryGraphicsCmdBuffers_[i].bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);

    primaryGraphicsCmdBuffers_[i].bindVertexBuffers(0, static_cast<vk::Buffer>(vertexBuffer_), static_cast<VkDeviceSize>(0ul));
    primaryGraphicsCmdBuffers_[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayoutData_.layout, 0,
                                                     static_cast<vk::DescriptorSet>(descriptorSets_[i]));

    primaryGraphicsCmdBuffers_[i].draw(3, 1, 0, 0);
    primaryGraphicsCmdBuffers_[i].endRenderPass();
    primaryGraphicsCmdBuffers_[i].end();
  }
}

void HelloTriangle::onViewChanged() {
  updateMatrixBuffers();
}

void HelloTriangle::onSwapChainRecreate() {
  createFrameBuffers();
  createGraphicalPipeline();
  recordCommandBuffers();
}

void HelloTriangle::initialize() {
  zoom = -2.5f;

  loadShaders();
  allocateBuffers();
  initializeDescriptorSets();

  createRenderPass();
  createFrameBuffers();
  createGraphicalPipeline();
  recordCommandBuffers();
}

void HelloTriangle::draw() {}
