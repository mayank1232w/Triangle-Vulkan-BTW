#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "Utils.h"
#include "Device.h"
#include "../resources/Depth.h"

struct Swapchain
{
	Device* Device;

    VkSwapchainKHR Swapchain;
    VkSurfaceFormatKHR SurfaceFormat;

    VkExtent2D Extent;

    std::vector<VkImage> SwapchainImage;
    std::vector<VkImageView> ImageViews;

};

void createSwapchain(Swapchain* swapchain);
void createImageViews(Swapchain* swapchain);
void recreateSwapchain(Swapchain* swapchain, Depth* depth);
void destroySwapchain(Swapchain* swapchain);

#endif // !SWAPCHAIN_H

#ifdef SWAPCHAIN_IMPLEMENTATION
#undef SWAPCHAIN_IMPLEMENTATION

void recreateSwapchain(Swapchain* swapchain, Depth* depth)
{
    vkDeviceWaitIdle(swapchain->Device->LogicalDevice);
    destroySwapchain(swapchain);   
    destoryDepthBuffer(depth, swapchain->Device);
    createSwapchain(swapchain);
    createImageViews(swapchain);
    createDepthBuffer(depth, swapchain->Device, swapchain->Extent);
}

void createSwapchain(Swapchain* swapchain)
{
    VkSurfaceCapabilitiesKHR surfaceCaps{};
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(swapchain->Device->PhysicalDevice, swapchain->Device->Surface, &surfaceCaps));

    uint32_t formatCount{};
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(swapchain->Device->PhysicalDevice, swapchain->Device->Surface, &formatCount, nullptr));
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(swapchain->Device->PhysicalDevice, swapchain->Device->Surface, &formatCount, formats.data()));

    for (uint32_t i = 0; i < formatCount; i++)
    {
        VkSurfaceFormatKHR format = formats[i];

        if (format.format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            swapchain->SurfaceFormat = format;
            break;
        }
    }

    VkSwapchainCreateInfoKHR createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = swapchain->Device->Surface;
    createInfo.minImageCount = surfaceCaps.minImageCount + 1 > surfaceCaps.maxImageCount ? surfaceCaps.minImageCount : surfaceCaps.minImageCount + 1;
    createInfo.imageFormat = swapchain->SurfaceFormat.format;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchain->Extent = surfaceCaps.currentExtent;

    createInfo.imageExtent = swapchain->Extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = surfaceCaps.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;

    VK_CHECK(vkCreateSwapchainKHR(swapchain->Device->LogicalDevice, &createInfo, nullptr, &swapchain->Swapchain));

    uint32_t swapchainImageCount{};
    VK_CHECK(vkGetSwapchainImagesKHR(swapchain->Device->LogicalDevice, swapchain->Swapchain, &swapchainImageCount, nullptr));

    swapchain->SwapchainImage.resize(swapchainImageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(swapchain->Device->LogicalDevice, swapchain->Swapchain, &swapchainImageCount, swapchain->SwapchainImage.data()));
}

void createImageViews(Swapchain* swapchain)
{
    VkImageViewCreateInfo createInfo{};


    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = swapchain->SurfaceFormat.format;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.layerCount = 1;
    createInfo.subresourceRange.levelCount = 1;

    swapchain->ImageViews.resize(swapchain->SwapchainImage.size());

    for (uint32_t i = 0; i < swapchain->ImageViews.size(); i++)
    {
        createInfo.image = swapchain->SwapchainImage[i];

        VK_CHECK(vkCreateImageView(swapchain->Device->LogicalDevice, &createInfo, nullptr, &swapchain->ImageViews[i]));
    }
}

void destroySwapchain(Swapchain* swapchain)
{
    for (auto imageView : swapchain->ImageViews)
    {
        vkDestroyImageView(swapchain->Device->LogicalDevice, imageView, nullptr);
    }
    vkDestroySwapchainKHR(swapchain->Device->LogicalDevice, swapchain->Swapchain, nullptr);
}

#endif // !SWAPCHAIN_IMPLEMENTATION