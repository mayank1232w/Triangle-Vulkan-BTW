#ifndef SYNC_H
#define SYNC_H

#include "Utils.h"
#include "Device.h"

struct Sync
{
    Device* Device;
	VkSemaphore AcquireSemaphore;
	VkSemaphore SubmitSemaphore;
};

void createSemaphore(Sync* sync);
void destroySync(Sync* sync);

#endif // !SYNC_H

#ifdef SYNC_IMPLEMENTATION
#undef SYNC_IMPLEMENTATION

void createSemaphore(Sync* sync) 
{
    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VK_CHECK(vkCreateSemaphore(sync->Device->LogicalDevice, &createInfo, nullptr, &sync->SubmitSemaphore));
    VK_CHECK(vkCreateSemaphore(sync->Device->LogicalDevice, &createInfo, nullptr, &sync->AcquireSemaphore));
}

void destroySync(Sync* sync)
{
    vkDestroySemaphore(sync->Device->LogicalDevice, sync->AcquireSemaphore, nullptr);
    vkDestroySemaphore(sync->Device->LogicalDevice, sync->SubmitSemaphore, nullptr);
}

#endif // !SYNC_IMPLEMENTATION