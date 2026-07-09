#ifndef DEVICE_H
#define DEVICE_H

#include "Utils.h"

struct Device
{
    VkInstance Instance;
    GLFWwindow* Window;
    VkSurfaceKHR Surface;
    VkPhysicalDevice PhysicalDevice;

    u32 GraphicsIdx;
    VkDevice LogicalDevice;
    VkQueue GraphicsQueue;
};

void createSurface(Device* device);
void pickGPU(Device* device);
void getQueueIndex(Device* device);
void createLogicalDevice(Device* device);

VkDevice initDevice(Device* device, VkInstance instance, GLFWwindow* window);

void destroyDevice(Device* device);

#endif // !DEVICE_H

#ifdef DEVICE_IMPLEMENTATION
#undef DEVICE_IMPLEMENTATION

VkDevice initDevice(Device* device, VkInstance instance, GLFWwindow* window)
{
    device->Instance = instance;
    device->Window = window;
    createSurface(device);
    pickGPU(device);
    getQueueIndex(device);
    createLogicalDevice(device);

    return device->LogicalDevice ;
}

void createSurface(Device* device)
{
    VK_CHECK(glfwCreateWindowSurface(device->Instance, device->Window, nullptr, &device->Surface));
}

void pickGPU(Device* device)
{
    uint32_t physicalDeviceCount;
    VK_CHECK(vkEnumeratePhysicalDevices(device->Instance, &physicalDeviceCount, nullptr));
    std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(device->Instance, &physicalDeviceCount, devices.data()));

    for (uint32_t i = 0; i < physicalDeviceCount; ++i)
    {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(devices[i], &props);

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            device->PhysicalDevice = devices[i];
            return;
        }
    }
}

void getQueueIndex(Device* device)
{
    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device->PhysicalDevice, &queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> queues(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device->PhysicalDevice, &queueCount, queues.data());

    for (uint32_t i = 0; i < queueCount; i++)
    {
        if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            VkBool32 surfaceSupport{};
            VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device->PhysicalDevice, i, device->Surface, &surfaceSupport));

            if (surfaceSupport)
            {
                device->GraphicsIdx = i;
                return;
            }
        }
    }
}

void createLogicalDevice(Device* device)
{
    float priorities = 1.0f;

    VkPhysicalDeviceFeatures features = {};
    vkGetPhysicalDeviceFeatures(device->PhysicalDevice, &features);

    const char* enabledExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceQueueCreateInfo queueCreateInfo{};

    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = device->GraphicsIdx;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &priorities;

    VkDeviceCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;

    createInfo.enabledExtensionCount = ARRAY_SIZE(enabledExtensions);
    createInfo.ppEnabledExtensionNames = enabledExtensions;
    createInfo.pEnabledFeatures = &features;

    VK_CHECK(vkCreateDevice(device->PhysicalDevice, &createInfo, nullptr, &device->LogicalDevice));
    vkGetDeviceQueue(device->LogicalDevice, device->GraphicsIdx, 0, &device->GraphicsQueue);
}

void destroyDevice(Device* device)
{
    vkDestroyDevice(device->LogicalDevice, nullptr);
    vkDestroySurfaceKHR(device->Instance, device->Surface, nullptr);
}

#endif // !DEVICE_IMPLEMENTATION
