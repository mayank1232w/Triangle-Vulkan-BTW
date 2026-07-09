#ifndef RENDERPASS_H
#define RENDERPASS_H

#include "Utils.h"
#include "Swapchain.h"

struct RenderPass
{
    Swapchain* Swapchain;
    Device* Device;
	VkRenderPass RenderPass;
};

void createRenderPass(RenderPass* renderPass);
void destroyRenderPass(RenderPass* renderPass);

#endif // !RENDERPASS_H

#ifdef RENDERPASS_IMPLEMENTATION
#undef RENDERPASS_IMPLEMENTATION

void createRenderPass(RenderPass* renderPass) 
{
    
    VkAttachmentDescription attachmentInfo{};
    attachmentInfo.format = renderPass->Swapchain->SurfaceFormat.format;
    attachmentInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentInfo.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentInfo{};
    colorAttachmentInfo.attachment = 0;
    colorAttachmentInfo.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef{};
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subPassInfo{};

    subPassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPassInfo.colorAttachmentCount = 1;
    subPassInfo.pColorAttachments = &colorAttachmentInfo;
    subPassInfo.pDepthStencilAttachment = &depthRef;

    VkAttachmentDescription attachments[] = { attachmentInfo, depthAttachment };

    VkRenderPassCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 2;
    createInfo.pAttachments = attachments;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subPassInfo;

    VK_CHECK(vkCreateRenderPass(renderPass->Device->LogicalDevice, &createInfo, nullptr, &renderPass->RenderPass));
}

void destoryRenderPass(RenderPass* renderPass)
{
    vkDestroyRenderPass(renderPass->Device->LogicalDevice, renderPass->RenderPass, nullptr);
}

#endif // !RENDERPASS_IMPLEMENTATION
