#ifndef COMMANDPOOL_H
#define COMMANDPOOL_H

#include "Utils.h"
#include "Device.h"

struct CommandPool
{
    Device* Device;
    VkCommandPool CommandPool;
};

void createCommandPool(CommandPool* commandPool);
void destroyCommandPool(CommandPool* commandPool);

#endif // !COMMANDPOOL_H

#ifdef COMMANDPOOL_IMPLEMENTATION
#undef COMMANDPOOL_IMPLEMENTATION

void createCommandPool(CommandPool* commandPool) 
{
    VkCommandPoolCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = commandPool->Device->GraphicsIdx;
    VK_CHECK(vkCreateCommandPool(commandPool->Device->LogicalDevice, &createInfo, nullptr, &commandPool->CommandPool));
}

void destroyCommandPool(CommandPool* commandPool) 
{
    vkDestroyCommandPool(commandPool->Device->LogicalDevice, commandPool->CommandPool, nullptr);
}

#endif // !COMMANDPOOL_IMPLEMENTATION