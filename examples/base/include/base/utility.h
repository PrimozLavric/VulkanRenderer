#ifndef UTILITY_H
#define UTILITY_H

#include "logi/logi.hpp"
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>


namespace utility
{   
    //----------Region: Vulkan state-------------------
    struct VulkanState
    {
        std::map<std::string, logi::LogicalDevice> logicalDevices;
        std::map<std::string, logi::MemoryAllocator> allocators;
        std::map<std::string, logi::Queue> queues;
        std::map<std::string, logi::CommandPool> commandPools;

        logi::LogicalDevice* defaultLogicalDevice;
        logi::MemoryAllocator* defaultAllocator;
        logi::Queue* defaultGraphicsQueue;
        logi::Queue* defaultComputeQueue;
        logi::Queue* defaultPresentQueue;
        logi::CommandPool* defaultCommandPool;

        VulkanState() : defaultLogicalDevice(nullptr), defaultAllocator(nullptr), defaultGraphicsQueue(nullptr),
                            defaultComputeQueue(nullptr), defaultPresentQueue(nullptr), defaultCommandPool(nullptr) 
        {}

        void addLogicalDevice(std::string& key, logi::LogicalDevice& logicalDevice)
        {
            // Returns iterator if key is already present in map
            auto const result = logicalDevices.insert(std::pair<std::string, logi::LogicalDevice>(key, logicalDevice));
            if(!result.second) {result.first->second = logicalDevice;} // Update if key already exists
        }

        void addAllocator(std::string& key, logi::MemoryAllocator& allocator)
        {
            auto const result = allocators.insert(std::pair<std::string, logi::MemoryAllocator>(key, allocator));
            if(!result.second) {result.first->second = allocator;}
        }

        void addQueue(std::string& key, logi::Queue& queue) 
        {
            auto const result = queues.insert(std::pair<std::string, logi::Queue>(key, queue));
            if(!result.second) {result.first->second = queue;}
        }

        void addCommandPool(std::string& key, logi::CommandPool& commandPool) 
        {
            auto const result = commandPools.insert(std::pair<std::string, logi::CommandPool>(key, commandPool));
            if(!result.second) {result.first->second = commandPool;}
        }

        void setDefaultLogicalDevice(std::string& key)
        {
            defaultLogicalDevice = &logicalDevices.at(key);
        }

        void setDefaultAllocator(std::string& key) 
        {
            defaultAllocator = &allocators.at(key);
        }

        void setDefaultGraphicsQueue(std::string& key) 
        {
            defaultGraphicsQueue = &queues.at(key);
        }

        void setDefaultComputeQueue(std::string& key) 
        {
            defaultComputeQueue = &queues.at(key);
        }

        void setDefaultPresentQueue(std::string& key) 
        {
            defaultPresentQueue = &queues.at(key);
        }        

        void setDefaultCommandPool(std::string& key) 
        {
            defaultCommandPool = &commandPools.at(key);
        }

    };


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

    logi::ShaderModule createShaderModule(const logi::LogicalDevice& logicalDevice, const std::string& shaderPath);
    ShaderReflection loadShaders(const logi::LogicalDevice& logicalDevice, 
                                 const std::string& vertexPath, const std::string& fragmentPath);
    PipelineLayoutData createPipelineLayout(const logi::LogicalDevice& logicalDevice, const ShaderReflection&);  


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

    void allocateBuffer(logi::MemoryAllocator& allocator, const VmaMemoryUsage& allocatorUsage,
                        std::vector<BufferAllocateInfo>& bufferAllocateInfos, std::vector<logi::VMABuffer>& bufferHandles);

    void allocateBufferStaged(logi::Queue& queue, logi::CommandPool& commandPool, logi::MemoryAllocator& allocator, 
                              const VmaMemoryUsage& allocatorUsage, std::vector<BufferAllocateInfo>& bufferAllocateInfos,
                              std::vector<logi::VMABuffer>& bufferHandles);


    //----------Region: image manipulation----------
    logi::VMAImage loadImage(logi::MemoryAllocator allocator, const VmaMemoryUsage& memoryUsage, 
                             const vk::ImageUsageFlagBits& usage, const char* path, const uint32_t& mipLevels = 1, 
                             const vk::SampleCountFlagBits& sampleCount = vk::SampleCountFlagBits::e1);

    logi::VMAImage loadImageStaged(logi::MemoryAllocator allocator, const VmaMemoryUsage& memoryUsage, 
                                   const vk::ImageUsageFlagBits& usage, const vk::ImageLayout& layout, const char* path,
                                   logi::CommandPool& commandPool, logi::Queue& queue, const uint32_t& mipLevels = 1, 
                                   const vk::SampleCountFlagBits& sampleCount = vk::SampleCountFlagBits::e1);

    logi::VMAImage createImage(logi::MemoryAllocator& allocator, const uint32_t& width, const uint32_t& height, const vk::Format& format,
                               const vk::ImageUsageFlags& usage, const VmaMemoryUsage& memoryUsage, const uint32_t& mipLevels = 1, 
                               const vk::SampleCountFlagBits& sampleCount = vk::SampleCountFlagBits::e1);

    void transitionImageLayout(logi::VMAImage& image, const vk::ImageLayout& oldLayout, const vk::ImageLayout& newLayout,
                               logi::CommandPool& commandPool, logi::Queue& queue, const uint32_t& mipLevel = 1);

    void copyBufferToImage(logi::VMABuffer& buffer, logi::Image& image, const uint32_t& width, const uint32_t& height, 
                           logi::CommandPool& commandPool, logi::Queue& queue);


    //----------Region: common command buffer patterns----------
    logi::CommandBuffer beginSingleTimeCommand(logi::CommandPool& commandPool);
    void endSingleTimeCommand(logi::Queue& queue, logi::CommandBuffer& commandBuffer);


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