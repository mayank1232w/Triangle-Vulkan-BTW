#ifndef INSTANCE_H
#define INSTANCE_H

#include "Utils.h"

struct Instance
{
    std::vector<const char*> ValidationLayer = { "VK_LAYER_KHRONOS_validation"};
    VkInstance Instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    bool enableDebug;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

void createInstance(Instance* instance);
VkInstance initInstance(Instance* instance);
void destroyInstance(Instance* instance);

#endif // INSTANCE_H

#ifdef INSTANCE_IMPLEMENTATION
#undef INSTANCE_IMPLEMENTATION

VkInstance initInstance(Instance* instance)
{
    createInstance(instance);
    return instance->Instance;
}

bool checkValidationLayerSupport(Instance* instance)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::vector<const char*> validation = instance->ValidationLayer;

    for (const char* layerName : validation)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        fprintf(stderr, "[Validation Layer]: %s\n\n", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

void createInstance(Instance* instance) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    VkApplicationInfo appInfo{};

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "IDK";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    if (instance->enableDebug)
    {
        if (checkValidationLayerSupport(instance))
        {
            createInfo.enabledLayerCount = (u32)instance->ValidationLayer.size();
            createInfo.ppEnabledLayerNames = instance->ValidationLayer.data();
        }
    }
    
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance->Instance));

    auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance->Instance, "vkCreateDebugUtilsMessengerEXT");
    if (vkCreateDebugUtilsMessengerEXT)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugInfo.pfnUserCallback = debugCallBack;

        VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance->Instance, &debugInfo, nullptr, &instance->debugMessenger));
    }

}

VkInstance getInstance(Instance* instance)
{
    if (instance->Instance == VK_NULL_HANDLE) {
        fprintf(stderr, "[WARNING]: Instance Handle is a Nullptr");
        return nullptr;
    }
    return instance->Instance;
}

void destroyInstance(Instance* instance)
{
    if (instance->enableDebug)
    {
        auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance->Instance, "vkDestroyDebugUtilsMessengerEXT");
        if (vkDestroyDebugUtilsMessengerEXT)
        {
            vkDestroyDebugUtilsMessengerEXT(instance->Instance, instance->debugMessenger, nullptr);
        }
    }

    vkDestroyInstance(instance->Instance, nullptr);
}

#endif // !INSTANCE_IMPLEMENTATION