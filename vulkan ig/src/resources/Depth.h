#ifndef DEPTH_H
#define DEPTH_H

#include "../core/Device.h"
#include "../core/Buffers.h"


struct Depth
{
	VkImage Image;
    VkDeviceMemory Memory;
    VkImageView ImageView;
};

void createDepthBuffer(Depth* depth, Device* device, VkExtent2D extent);
void destoryDepthBuffer(Depth* depth, Device* device);


#endif // !DEPTH_H
#ifdef DEPTH_IMPLEMENTATION
#undef DEPTH_IMPLEMENTATION

void createDepthBuffer(Depth* depth, Device* device, VkExtent2D extent)
{
	VkImageCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.format = VK_FORMAT_D32_SFLOAT;
    createInfo.extent = { extent.width, extent.height, 1 };
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 1;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VK_CHECK(vkCreateImage(device->LogicalDevice, &createInfo, nullptr, &depth->Image));

    VkMemoryRequirements memReq{};
    vkGetImageMemoryRequirements(device->LogicalDevice, depth->Image, &memReq);

    VkMemoryAllocateInfo allocInfo{};


    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(device, memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkAllocateMemory(device->LogicalDevice, &allocInfo, nullptr, &depth->Memory));
    VK_CHECK(vkBindImageMemory(device->LogicalDevice, depth->Image, depth->Memory, 0));

    VkImageViewCreateInfo imageInfo{};

    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageInfo.image = depth->Image;
    imageInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageInfo.format = VK_FORMAT_D32_SFLOAT;
    imageInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imageInfo.subresourceRange.levelCount = 1;
    imageInfo.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(device->LogicalDevice, &imageInfo, nullptr, &depth->ImageView));
}

void destoryDepthBuffer(Depth* depth, Device* device)
{
    vkDestroyImageView(device->LogicalDevice, depth->ImageView, nullptr);
    vkDestroyImage(device->LogicalDevice, depth->Image, nullptr);
    vkFreeMemory(device->LogicalDevice, depth->Memory, nullptr);
}

#endif // !DEPTH_IMPLEMENTATION
