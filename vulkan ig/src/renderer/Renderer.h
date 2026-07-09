#ifndef RENDERER_H
#define RENDERER_H

#include "../core/Device.h"
#include "../core/Swapchain.h"
#include "../core/RenderPass.h"
#include "../core/Framebuffer.h"
#include "../core/CommandPool.h"
#include "../core/Sync.h"
#include "../pipeline/Pipeline.h"
#include "../resources/Mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>


struct Renderer {
    Device* Device;
    Swapchain* Swapchain;
    RenderPass* RenderPass;
    Framebuffer* Framebuffer;
    CommandPool* CommandPool;
    Sync* Sync;
    Pipeline* Pipeline;
    Mesh* Mesh;
};

VkCommandBuffer recordCommandBuffer(Renderer* renderer, uint32_t imageIndex);
void render(Renderer* renderer);

#endif // !RENDERER_H

#ifdef RENDERER_IMPLEMENTATION
#undef RENDERER_IMPLEMENTATION


VkCommandBuffer recordCommandBuffer(Renderer* renderer, uint32_t imageIndex)
{
    VkExtent2D extent = renderer->Swapchain->Extent;

    VkCommandBuffer cmd;
    VkCommandBufferAllocateInfo cmdInfo{};
    cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdInfo.commandPool = renderer->CommandPool->CommandPool;
    cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdInfo.commandBufferCount = 1;
    VK_CHECK(vkAllocateCommandBuffers(renderer->Device->LogicalDevice, &cmdInfo, &cmd));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

    VkClearValue clearValue[2];
    clearValue[0].color = {1, 0, 0, 1};
    clearValue[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo rpBeginInfo{};
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.renderPass = renderer->RenderPass->RenderPass;
    rpBeginInfo.framebuffer = renderer->Framebuffer->frameBuffers[imageIndex];
    rpBeginInfo.renderArea.extent = { extent.width, extent.height};
    rpBeginInfo.clearValueCount = 2;
    rpBeginInfo.pClearValues = clearValue;

    vkCmdBeginRenderPass(cmd, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkRect2D scissor{};
    scissor.extent = { extent.width, extent.height };
    VkViewport viewport{};
    viewport.height = extent.height;
    viewport.width = extent.width;
    viewport.maxDepth = 1.0f;

    vkCmdSetScissor(cmd, 0, 1, &scissor);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    
    VkBuffer vertexBuffers[] = { renderer->Mesh->VertexBuffer.Buffer };
    VkDeviceSize offsets[] = { 0 };
    
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmd, renderer->Mesh->Indices.Buffer, 0, VK_INDEX_TYPE_UINT32);
    
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->Pipeline->Pipeline);

    static auto startTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - startTime).count();
    
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)extent.width / (float)extent.height, 0.1f, 10.0f);
    proj[1][1] *= -1; 

    glm::mat4 mvp = proj * view * model;

    vkCmdPushConstants(cmd, renderer->Pipeline->PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mvp);

    vkCmdDraw(cmd, renderer->Mesh->VertexCount, 1, 0, 0);
    vkCmdDrawIndexed(cmd, renderer->Mesh->IndicesCount, 1, 0, 0, 0);

    vkCmdEndRenderPass(cmd);
    VK_CHECK(vkEndCommandBuffer(cmd));

    return cmd;
}

void render(Renderer* renderer)
{
    if (renderer->Framebuffer->Window->Width == 0 || renderer->Framebuffer->Window->Height == 0) {
        return;
    }

    uint32_t imgIdx{};
    VkResult acquireResult = (vkAcquireNextImageKHR(renderer->Device->LogicalDevice, renderer->Swapchain->Swapchain,
        UINT64_MAX, renderer->Sync->AcquireSemaphore, 0, &imgIdx));

    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain(renderer->Swapchain, renderer->Framebuffer->Depth);
        recreateFramebuffer(renderer->Framebuffer);
        return;
    }

    VkCommandBuffer cmd = recordCommandBuffer(renderer, imgIdx);

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &renderer->Sync->AcquireSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderer->Sync->SubmitSemaphore;

    VK_CHECK(vkQueueSubmit(renderer->Device->GraphicsQueue, 1, &submitInfo, 0));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &renderer->Swapchain->Swapchain;
    presentInfo.pImageIndices = &imgIdx;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderer->Sync->SubmitSemaphore;

    VkResult presentResult = vkQueuePresentKHR(renderer->Device->GraphicsQueue, &presentInfo);

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || renderer->Framebuffer->Window->Resized) {
        renderer->Framebuffer->Window->Resized = false;
        recreateSwapchain(renderer->Swapchain, renderer->Framebuffer->Depth);
        recreateFramebuffer(renderer->Framebuffer);
    }
    else {
        VK_CHECK(presentResult);
    }
    VK_CHECK(vkDeviceWaitIdle(renderer->Device->LogicalDevice));
    vkFreeCommandBuffers(renderer->Device->LogicalDevice, renderer->CommandPool->CommandPool, 1, &cmd);
}

#endif // !RENDERER_IMPLEMENTATION