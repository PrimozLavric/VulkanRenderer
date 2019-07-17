#ifndef EXAMPLE_HELLO_TRIANGLE_H
#define EXAMPLE_HELLO_TRIANGLE_H
#include <glm/glm.hpp>
#include "base/example_base.h"

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;
};

class HelloTriangle : public ExampleBase {
 protected:
  void loadShaders();

  void allocateBuffers();

  void updateMatrixBuffers();

  void initializeDescriptorSets();

  void createRenderPass();

  void createGraphicalPipeline();

  void createFrameBuffers();

  void recordCommandBuffers();

  void onViewChanged() override;

  void onSwapChainRecreate() override;

  void initialize() override;

  void draw() override;

 private:
  struct {
    glm::mat4 projectionMatrix = glm::mat4(1);
    glm::mat4 modelMatrix = glm::mat4(1);
    glm::mat4 viewMatrix = glm::mat4(1);
  } uboMatrices;

  static constexpr uint32_t kVertexBinding = 0u;
  std::vector<Vertex> vertices = {{{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                  {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                  {{0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

  logi::MemoryAllocator allocator_;
  logi::DescriptorPool descriptorPool_;

  logi::RenderPass renderPass_;
  logi::ShaderModule vertexShader_;
  logi::ShaderModule fragmentShader_;

  PipelineLayoutData pipelineLayoutData_;

  logi::VMABuffer vertexBuffer_;
  std::vector<logi::VMABuffer> matricesUBOBuffers_;

  std::vector<logi::DescriptorSet> descriptorSets_;
  logi::Pipeline pipeline_;
  std::vector<logi::Framebuffer> framebuffers_;
};

#endif