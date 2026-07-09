#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "Utils.h"

#include "RenderPass.h"
#include "Window.h"
#include "Swapchain.h"
#include "Device.h"
#include "../resources/Depth.h"

struct Framebuffer 
{
    Window* Window;
    RenderPass* RenderPass;
    Swapchain* SwapChain;
    Device* Device;
    Depth* Depth;

    std::vector<VkFramebuffer> frameBuffers;
};

void createFrameBuffer(Framebuffer* framebuffer);
void destroyFramebuffer(Framebuffer* framebuffer);
void recreateFramebuffer(Framebuffer* framebuffer);

#endif // !FRAMEBUFFER_H

#ifdef FRAMEBUFFER_IMPLEMENTATION
#undef FRAMEBUFFER_IMPLEMENTATION

void recreateFramebuffer(Framebuffer* framebuffer)
{
    destroyFramebuffer(framebuffer);
    createFrameBuffer(framebuffer);
}

void createFrameBuffer(Framebuffer* framebuffer)
{
    
    VkFramebufferCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = framebuffer->RenderPass->RenderPass;
    createInfo.width = framebuffer->Window->Width;
    createInfo.height = framebuffer->Window->Height;
    createInfo.layers = 1;
    
    framebuffer->frameBuffers.resize(framebuffer->SwapChain->SwapchainImage.size());

    for (uint32_t i = 0; i < framebuffer->SwapChain->SwapchainImage.size(); i++)
    {
        VkImageView attachments[] = {framebuffer->SwapChain->ImageViews[i], framebuffer->Depth->ImageView};
        createInfo.pAttachments = attachments;
        createInfo.attachmentCount = 2;
        VK_CHECK(vkCreateFramebuffer(framebuffer->Device->LogicalDevice, &createInfo, nullptr, &framebuffer->frameBuffers[i]));
    }
}

void destroyFramebuffer(Framebuffer* framebuffer)
{
    for (auto fb : framebuffer->frameBuffers)
    {
        vkDestroyFramebuffer(framebuffer->Device->LogicalDevice, fb, nullptr);
    }
}

#endif // !FRAMEBUFFER_IMPLEMENTATION