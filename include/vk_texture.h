/*
* Project       : VulkanRenderer
*
* Author        : Primoz Lavric
*
* For the terms and conditions of redistribution or use of this software
* refer to the file LICENSE located in the root level directory
* of the project.
*/

#ifndef _VKR_VK_TEXTURE_H_
#define _VKR_VK_TEXTURE_H_

#include <vulkan/vulkan.h>

class VulkanTexture {
public:
	struct TextureConfiguration {

	};

	VulkanTexture(VkImage image);
	~VulkanTexture();

private:

};

#endif //  _VKR_VK_TEXTURE_H_