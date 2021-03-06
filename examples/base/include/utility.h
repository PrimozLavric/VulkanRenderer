#ifndef UTILITY_H
#define UTILITY_H

#include "logi/logi.hpp"
#include "vulkanState.h"
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>


namespace utility
{   
    enum PipelineType {Graphics, Compute, Present};

    //----------Region: Shader Reflection----------
    struct ShaderReflection 
    {   
        explicit ShaderReflection(logi::ShaderModule vertexShader = {},
                                    logi::ShaderModule fragmentShader = {},
                                    std::vector<logi::DescriptorSetReflectionInfo> descriptorSetInfo = {},
                                    std::vector<logi::PushConstantReflectionInfo> pushConstants = {});

        logi::ShaderModule vertexShader;
        logi::ShaderModule fragmentShader;
        std::vector<logi::DescriptorSetReflectionInfo> descriptorSetInfo;
        std::vector<logi::PushConstantReflectionInfo> pushConstants;
    };

    struct PipelineLayoutData {
        explicit PipelineLayoutData(logi::PipelineLayout layout = {},
                                    std::vector<logi::DescriptorSetLayout> descriptorSetLayouts = {});
        ;

        logi::PipelineLayout layout;
        std::vector<logi::DescriptorSetLayout> descriptorSetLayouts;
    };  

    logi::ShaderModule createShaderModule(const VulkanState& vulkanState, const std::string& shaderPath);
    ShaderReflection loadShaders(const VulkanState& vulkanState, 
                                 const std::string& vertexPath, const std::string& fragmentPath);
    PipelineLayoutData createPipelineLayout(const VulkanState& vulkanState, const ShaderReflection& shaderReflection);  


    //----------Region: model loading----------
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texCord;

        // Compare operator for unordered map
        bool operator==(const Vertex& other) const {
            return position == other.position && color == other.color && texCord == other.texCord;
        }
    };

    struct ModelObj
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;    
    };

    // TODO: extend obj loader
    void loadObjModel(const std::string& modelPath, ModelObj& model);  

    // TODO: gltw loading


    //----------Region: buffer creation----------
    struct BufferAllocateInfo
    {   
        void* data;
        vk::DeviceSize size;
        vk::BufferUsageFlags usage;
        vk::SharingMode sharingMode;
    };

    void allocateBuffer(const VulkanState& vulkanState, const VmaMemoryUsage& allocatorUsage,
                        std::vector<BufferAllocateInfo>& bufferAllocateInfos, std::vector<logi::VMABuffer>& bufferHandles);

    void allocateBufferStaged(const VulkanState& vulkanState, const VmaMemoryUsage& allocatorUsage, 
                              std::vector<BufferAllocateInfo>& bufferAllocateInfos, std::vector<logi::VMABuffer>& bufferHandles);


    //----------Region: image manipulation----------
    logi::VMAImage loadImage(const VulkanState& vulkanState, const VmaMemoryUsage& memoryUsage, 
                            const vk::ImageUsageFlagBits& usage, const char* path, const uint32_t& mipLevels = 1, 
                            const vk::SampleCountFlagBits& sampleCount = vk::SampleCountFlagBits::e1);

    logi::VMAImage loadImageStaged(const VulkanState& vulkanState, const VmaMemoryUsage& memoryUsage, 
                                   const vk::ImageUsageFlagBits& usage, const vk::ImageLayout& layout, const char* path,
                                   const uint32_t& mipLevels = 1, const vk::SampleCountFlagBits& sampleCount = vk::SampleCountFlagBits::e1);

    logi::VMAImage createImage(const VulkanState& vulkanState, const uint32_t& width, const uint32_t& height, const vk::Format& format,
                               const vk::ImageUsageFlags& usage, const VmaMemoryUsage& memoryUsage, const uint32_t& mipLevels = 1, 
                               const vk::SampleCountFlagBits& sampleCount = vk::SampleCountFlagBits::e1);

    void transitionImageLayout(const VulkanState& vulkanState, logi::VMAImage& image, const vk::ImageLayout& oldLayout,
                               const vk::ImageLayout& newLayout, const uint32_t& mipLevel = 1);

    void copyBufferToImage(const VulkanState& vulkanState, logi::VMABuffer& buffer, 
                           logi::Image& image, const uint32_t& width, const uint32_t& height);


    //----------Region: common command buffer patterns----------
    logi::CommandBuffer beginSingleTimeCommand(const VulkanState& vulkanState, const utility::PipelineType& queueType);
    logi::CommandBuffer getGraphicsCommand(const VulkanState& vulkanState);
    logi::CommandBuffer getComputeCommand(const VulkanState& vulkanState);
    logi::CommandBuffer getPresentCommand(const VulkanState& vulkanState);
    
    void endSingleTimeCommand(const VulkanState& vulkanState, const utility::PipelineType& queueType, logi::CommandBuffer& commandBuffer);
    void submitGraphicsCommand(const VulkanState& vulkanState, const vk::SubmitInfo& submitInfo);
    void submitComputeCommand(const VulkanState& vulkanState, const vk::SubmitInfo& submitInfo);
    void submitPresentCommand(const VulkanState& vulkanState, const vk::SubmitInfo& submitInfo);



    // //----------Region: framebuffer----------
    // struct FramebufferAttachmentCreateInfo {
    //     uint32_t width; 
    //     uint32_t height; 
    //     vk::Format format;
    //     vk::ImageUsageFlags usage;
    //     VmaMemoryUsage& memoryUsage;
    // };

    // struct FramebufferAttachment {
    //     logi::VMAImage image;
    //     logi::ImageView imageView;
    //     vk::Format format;
    //     vk::AttachmentDescription attachemntDescription;

    //     // vk::ImageSubresourceRange subresourceRange;

	// 	bool hasDepth()
	// 	{
	// 		std::vector<VkFormat> formats = 
	// 		{
	// 			VK_FORMAT_D16_UNORM,
	// 			VK_FORMAT_X8_D24_UNORM_PACK32,
	// 			VK_FORMAT_D32_SFLOAT,
	// 			VK_FORMAT_D16_UNORM_S8_UINT,
	// 			VK_FORMAT_D24_UNORM_S8_UINT,
	// 			VK_FORMAT_D32_SFLOAT_S8_UINT,
	// 		};
	// 		return std::find(formats.begin(), formats.end(), format) != std::end(formats);
	// 	}

	// 	bool hasStencil()
	// 	{
	// 		std::vector<VkFormat> formats = 
	// 		{
	// 			VK_FORMAT_S8_UINT,
	// 			VK_FORMAT_D16_UNORM_S8_UINT,
	// 			VK_FORMAT_D24_UNORM_S8_UINT,
	// 			VK_FORMAT_D32_SFLOAT_S8_UINT,
	// 		};
	// 		return std::find(formats.begin(), formats.end(), format) != std::end(formats);
	// 	}

	// 	bool isDepthStencil()
	// 	{
	// 		return(hasDepth() || hasStencil());
	// 	}
    // };

    // struct Framebuffer {
    //     private:
    //         logi::MemoryAllocator allocator_;
        
    //     public:

    // };

} // namespace utility

// Hash function for Vertex used in unordered map
namespace std {
	template<> struct hash<utility::Vertex> {
		size_t operator()(utility::Vertex const& vertex) const {
		return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCord) << 1);
		}
	};
}

#endif