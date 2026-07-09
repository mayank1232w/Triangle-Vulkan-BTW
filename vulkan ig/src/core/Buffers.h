#ifndef BUFFERS_H
#define BUFFERS_H

#include "Utils.h"
#include "Device.h"
#include "CommandPool.h"

struct Buffer {
	VkBuffer Buffer;
	VkDeviceMemory bufferMemory;
	VkDeviceSize Size;
};

uint32_t findMemoryType(Device* device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void createBuffer(Buffer* buffer, Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
void destroyBuffer(Device* device, Buffer* buffer);
void copyBuffer(Device* device, CommandPool* commandPool, Buffer* src, Buffer* dst, VkDeviceSize size);


#endif // !BUFFERS_H

#ifdef BUFFERS_IMPLEMENTATION
#undef BUFFERS_IMPLEMENTATION

uint32_t findMemoryType(Device* device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProp{};
	vkGetPhysicalDeviceMemoryProperties(device->PhysicalDevice, &memProp);

	for (uint32_t i = 0; i < memProp.memoryTypeCount; i++)
	{
		bool isSupported = (typeFilter & (1 << i));
		bool hasRequiredFlags = (memProp.memoryTypes[i].propertyFlags & properties) == properties;

		if (isSupported && hasRequiredFlags)
		{
			return i;
		}
	}
	throw std::runtime_error("Failed to find suitable memory type");
}

void createBuffer(Buffer* buffer, Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
	buffer->Size = size;

	VkBufferCreateInfo createInfo{};

	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.usage = usage;

	VK_CHECK(vkCreateBuffer(device->LogicalDevice, &createInfo, nullptr, &buffer->Buffer));

	VkMemoryRequirements memReq{};
	vkGetBufferMemoryRequirements(device->LogicalDevice, buffer->Buffer, &memReq);

	
	VkMemoryAllocateInfo allocInfo{};

	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findMemoryType(device, memReq.memoryTypeBits, properties);

	VK_CHECK(vkAllocateMemory(device->LogicalDevice, &allocInfo, nullptr, &buffer->bufferMemory));
	VK_CHECK(vkBindBufferMemory(device->LogicalDevice, buffer->Buffer, buffer->bufferMemory, 0));
}

void copyBuffer(Device* device, CommandPool* commandPool, Buffer* src, Buffer* dst, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool->CommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer cmd;
	VK_CHECK(vkAllocateCommandBuffers(device->LogicalDevice, &allocInfo, &cmd));

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(cmd, src->Buffer, dst->Buffer, 1, &copyRegion);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;

	VK_CHECK(vkQueueSubmit(device->GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK(vkQueueWaitIdle(device->GraphicsQueue));

	vkFreeCommandBuffers(device->LogicalDevice, commandPool->CommandPool, 1, &cmd);
}

void destroyBuffer(Device* device, Buffer* buffer)
{
	vkDestroyBuffer(device->LogicalDevice, buffer->Buffer, nullptr);
	vkFreeMemory(device->LogicalDevice, buffer->bufferMemory, nullptr);
}

#endif // !BUFFERS_IMPLEMTATION
