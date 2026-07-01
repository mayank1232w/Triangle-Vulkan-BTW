#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>
#include <fstream>
#include <string>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define VK_CHECK(result)                         \
    if (result != VK_SUCCESS)                    \
    {                                            \
        fprintf(stderr, "[ERROR]: %d", result);  \
        __debugbreak();                          \
    }

#define WIDTH 800
#define HEIGHT 600

/*
*@ Context:
* Collection of all the handles
* provided by the vulkan func calls;
*/

struct Context {
	VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    GLFWwindow* window;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;

    uint32_t graphicsIdx;
    VkDevice logicalDevice;
    VkSwapchainKHR swapchain;
    VkSurfaceFormatKHR surfaceFormat;
    std::vector<VkImage> swapchainImage;
    VkQueue graphicsQueue;

    VkRenderPass renderPass;
    VkCommandPool commandPool;

    VkSemaphore submitSemaphore;
    VkSemaphore acquireSemaphore;

    std::vector<VkFramebuffer> frameBuffers;
    std::vector<VkImageView> imageViews;
    
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

const char* validationLayer[] = {
    "VK_LAYER_KHRONOS_validation",
};

// TODO: Check if the validation layer exists on the machine

/*
*@ CallBack:
* This is a specific func signature to call the vulkan
* and get the debug CallBack;
*/

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) 
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        fprintf(stderr, "[Validation Layer]: %s\n\n", pCallbackData->pMessage);
    }

    return VK_FALSE;
}
/*
*@ CreateInstance:
* Creates the instance for the vulkan
* also handles the extensions inside itself;
*/


void createInstance(Context* ctx) {
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
    createInfo.enabledLayerCount = ARRAY_SIZE(validationLayer);
    createInfo.ppEnabledLayerNames = validationLayer;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

	VK_CHECK(vkCreateInstance(&createInfo, nullptr, &ctx->instance));
    
    auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(ctx->instance, "vkCreateDebugUtilsMessengerEXT");
    if (vkCreateDebugUtilsMessengerEXT) {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugInfo.pfnUserCallback = debugCallBack;

        VK_CHECK(vkCreateDebugUtilsMessengerEXT(ctx->instance, &debugInfo, nullptr, &ctx->debugMessenger));
    }

}

/*
*@ CreateSurface:
* vulkan needs this surface for something IDK
* maybe it cant directly work on the window given by the glfw;
*/

void createSurface(Context* ctx) {
    VK_CHECK(glfwCreateWindowSurface(ctx->instance, ctx->window, nullptr, &ctx->surface));
}


/*
*@ PickGPU:
* Just picks the DiscreteGPU for now will change it to fallback 
* on the integrated if it cant find the discrete;
*/

void pickGPU(Context* ctx) {
    uint32_t physicalDeviceCount;
    VK_CHECK(vkEnumeratePhysicalDevices(ctx->instance, &physicalDeviceCount, nullptr));
    std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(ctx->instance, &physicalDeviceCount, devices.data()));

    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(devices[i], &props);
        
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            ctx->physicalDevice = devices[i];
            return;
        }
    }
}

/*
*@ getQueueIndex:
* Gets the queue index that supports the graphics and 
* presentation support;
*/

void getQueueIndex(Context* ctx) {
    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> queues(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &queueCount, queues.data());

    for (uint32_t i = 0; i < queueCount; i++) {
        if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            VkBool32 surfaceSupport{};
            VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(ctx->physicalDevice, i, ctx->surface, &surfaceSupport));

            if (surfaceSupport) {
                ctx->graphicsIdx = i;
                return;
            }
        }
    }
}

/*
*@ CreateLogicalDevice:
* makes the logical device out of the gpu picked
* it also gets the queue needed later;
*/

void createLogicalDevice(Context* ctx) {
    float priorities = 1.0f;

    VkPhysicalDeviceFeatures features = {};
    vkGetPhysicalDeviceFeatures(ctx->physicalDevice, &features);

    const char* enabledExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceQueueCreateInfo queueCreateInfo{};

    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = ctx->graphicsIdx;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &priorities;

    VkDeviceCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;

    createInfo.enabledExtensionCount = ARRAY_SIZE(enabledExtensions);
    createInfo.ppEnabledExtensionNames = enabledExtensions;
    createInfo.pEnabledFeatures = &features;

    VK_CHECK(vkCreateDevice(ctx->physicalDevice, &createInfo, nullptr, &ctx->logicalDevice));
    vkGetDeviceQueue(ctx->logicalDevice, ctx->graphicsIdx, 0, &ctx->graphicsQueue);
}

/*
*@ CreateSwapchain:
* creates the swapchain and swapchain images;
*/

void createSwapchain(Context* ctx){
    VkSurfaceCapabilitiesKHR surfaceCaps{};
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physicalDevice, ctx->surface, &surfaceCaps));

    uint32_t formatCount{};
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physicalDevice, ctx->surface, &formatCount, nullptr));
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physicalDevice, ctx->surface, &formatCount, formats.data()));

    for (uint32_t i = 0; i < formatCount; i++) {
        VkSurfaceFormatKHR format = formats[i];

        if (format.format == VK_FORMAT_B8G8R8A8_SRGB) {
            ctx->surfaceFormat = format;
            break;
        }
    }

    VkSwapchainCreateInfoKHR createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = ctx->surface;
    createInfo.minImageCount = surfaceCaps.minImageCount + 1 > surfaceCaps.maxImageCount ? surfaceCaps.minImageCount : surfaceCaps.minImageCount + 1;
    createInfo.imageFormat = ctx->surfaceFormat.format;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = surfaceCaps.currentExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = surfaceCaps.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;

    VK_CHECK(vkCreateSwapchainKHR(ctx->logicalDevice, &createInfo, nullptr, &ctx->swapchain));

    uint32_t swapchainImageCount{};
    VK_CHECK(vkGetSwapchainImagesKHR(ctx->logicalDevice, ctx->swapchain, &swapchainImageCount, nullptr));

    ctx->swapchainImage.resize(swapchainImageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(ctx->logicalDevice, ctx->swapchain, &swapchainImageCount, ctx->swapchainImage.data()));
}

/*
*@ CreateCommandPool:
* creates the CommandPool from where we will get the Command Buffers;
*/

void createCommandPool(Context* ctx) {
    VkCommandPoolCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = ctx->graphicsIdx;
    VK_CHECK(vkCreateCommandPool(ctx->logicalDevice, &createInfo, nullptr, &ctx->commandPool));
}

/*
*@ CreateSemaphore:
* creates the seamphores submit and aquire semaphore;
*/

void createSemaphore(Context* ctx) {
    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VK_CHECK(vkCreateSemaphore(ctx->logicalDevice, &createInfo, nullptr, &ctx->submitSemaphore));
    VK_CHECK(vkCreateSemaphore(ctx->logicalDevice, &createInfo, nullptr, &ctx->acquireSemaphore));
}

/*
*@ CreateSwapchain:
* creates the RenderPass;
*/

void createRenderPass(Context* ctx) {
    VkAttachmentDescription attachmentInfo{};
    attachmentInfo.format = ctx->surfaceFormat.format;
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

    VkSubpassDescription subPassInfo{};

    subPassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPassInfo.colorAttachmentCount = 1;
    subPassInfo.pColorAttachments = &colorAttachmentInfo;


    VkRenderPassCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &attachmentInfo;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subPassInfo;
    
    VK_CHECK(vkCreateRenderPass(ctx->logicalDevice, &createInfo, nullptr, &ctx->renderPass));
}

/*
*@ CreateImageViews:
* creates the swapchain and swapchain images;
*/

void createImageViews(Context* ctx) {
    VkImageViewCreateInfo createInfo{};


    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = ctx->surfaceFormat.format;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.layerCount = 1;
    createInfo.subresourceRange.levelCount = 1;

    ctx->imageViews.resize(ctx->swapchainImage.size());

    for (uint32_t i = 0; i < ctx->imageViews.size(); i++) {
        createInfo.image = ctx->swapchainImage[i];

        VK_CHECK(vkCreateImageView(ctx->logicalDevice, &createInfo, nullptr, &ctx->imageViews[i]));
    }
}


void createFrameBuffer(Context* ctx) {
    VkFramebufferCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = ctx->renderPass;
    createInfo.attachmentCount = 1;
    createInfo.width = WIDTH;
    createInfo.height = HEIGHT;
    createInfo.layers = 1;

    ctx->frameBuffers.resize(ctx->swapchainImage.size());

    for (uint32_t i = 0; i < ctx->swapchainImage.size(); i++) {
        createInfo.pAttachments = &ctx->imageViews[i];
        VK_CHECK(vkCreateFramebuffer(ctx->logicalDevice, &createInfo, nullptr, &ctx->frameBuffers[i]));
    }
}

std::vector<char> readFile(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open SPIR-V file: " + fileName);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void createPipelineLayout(Context* ctx) {
    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VK_CHECK(vkCreatePipelineLayout(ctx->logicalDevice, &createInfo, nullptr, &ctx->pipelineLayout));
}

void createPipeline(Context* ctx) {

    VkPipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.lineWidth = 1.0f;

    std::vector<char> vertexCode = readFile("assets/shader.vert.spv");
    std::vector<char> fragmentCode = readFile("assets/shader.frag.spv");

    VkShaderModule vertexShader, fragmentShader;

    VkShaderModuleCreateInfo vertexShaderInfo{};
    vertexShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertexShaderInfo.pCode = reinterpret_cast<const uint32_t*>(vertexCode.data());
    vertexShaderInfo.codeSize = vertexCode.size();

    VkShaderModuleCreateInfo fragmentShaderInfo{};
    fragmentShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragmentShaderInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentCode.data());
    fragmentShaderInfo.codeSize = fragmentCode.size();

    VK_CHECK(vkCreateShaderModule(ctx->logicalDevice, &vertexShaderInfo, nullptr, &vertexShader));
    VK_CHECK(vkCreateShaderModule(ctx->logicalDevice, &fragmentShaderInfo, nullptr, &fragmentShader));

    VkPipelineShaderStageCreateInfo vertexStage{};
    vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStage.pName = "main";
    vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStage.module = vertexShader;
    
    VkPipelineShaderStageCreateInfo fragmentStage{};
    fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStage.pName = "main";
    fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStage.module = fragmentShader;

    VkPipelineShaderStageCreateInfo shaderStage[] = {
        vertexStage,
        fragmentStage
    };

    VkPipelineVertexInputStateCreateInfo vertexInputState{};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    
    VkPipelineColorBlendAttachmentState colorAttachment{};
    colorAttachment.blendEnable = VK_FALSE;
    colorAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendState{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorAttachment;

    VkRect2D scissors{};
    VkViewport viewports{};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pScissors = &scissors;
    viewportState.pViewports = &viewports;
    viewportState.scissorCount = 1;
    viewportState.viewportCount = 1;

    VkDynamicState dynamicState[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.pDynamicStates = dynamicState;
    dynamicStateInfo.dynamicStateCount = ARRAY_SIZE(dynamicState);

    VkPipelineMultisampleStateCreateInfo multisampleState{};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;


    VkGraphicsPipelineCreateInfo createInfo{};

    createInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount = ARRAY_SIZE(shaderStage);
    createInfo.pStages    = shaderStage;
    createInfo.pVertexInputState = &vertexInputState;
    createInfo.renderPass = ctx->renderPass;
    createInfo.pColorBlendState = &colorBlendState;
    createInfo.pRasterizationState = &rasterizationState;
    createInfo.layout = ctx->pipelineLayout;
    createInfo.pDynamicState = &dynamicStateInfo;
    createInfo.pViewportState = &viewportState;
    createInfo.pMultisampleState = &multisampleState;
    createInfo.pInputAssemblyState = &inputAssemblyState;
    
    VK_CHECK(vkCreateGraphicsPipelines(ctx->logicalDevice, NULL, 1, &createInfo, nullptr, &ctx->pipeline));
    vkDestroyShaderModule(ctx->logicalDevice, vertexShader, nullptr);
    vkDestroyShaderModule(ctx->logicalDevice, fragmentShader, nullptr);
}

void render(Context* ctx) {
    uint32_t imgIdx{};
    VK_CHECK(vkAcquireNextImageKHR(ctx->logicalDevice, ctx->swapchain, UINT64_MAX, ctx->acquireSemaphore, 0, &imgIdx));

    VkCommandBuffer cmd;
    VkCommandBufferAllocateInfo cmdInfo{};

    cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdInfo.commandPool = ctx->commandPool;
    cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdInfo.commandBufferCount = 1;

    VK_CHECK(vkAllocateCommandBuffers(ctx->logicalDevice, &cmdInfo, &cmd));

    VkCommandBufferBeginInfo beginInfo{};

    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));
    
    VkClearValue clearValue{};
    clearValue.color = { 1, 1, 0, 1 };


    VkRenderPassBeginInfo renderPassBeginInfo{};
    
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = ctx->renderPass;
    renderPassBeginInfo.framebuffer = ctx->frameBuffers[imgIdx];
    renderPassBeginInfo.renderArea.extent = {WIDTH, HEIGHT};
    
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearValue;


    vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    {
        VkRect2D scissor{};
        scissor.extent = { WIDTH, HEIGHT };

        VkViewport viewport{};
        viewport.height = HEIGHT;
        viewport.width = WIDTH;
        viewport.maxDepth = 1.0f;

        vkCmdSetScissor(cmd, 0, 1, &scissor);
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pipeline);
        vkCmdDraw(cmd, 3, 1, 0, 0);
    }

    vkCmdEndRenderPass(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkPipelineStageFlags waitStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.pWaitDstStageMask = &waitStageFlags;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &ctx->acquireSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &ctx->submitSemaphore;

    VK_CHECK(vkQueueSubmit(ctx->graphicsQueue, 1, &submitInfo, 0));

    VkPresentInfoKHR presentInfo{};

    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &ctx->swapchain;
    presentInfo.pImageIndices = &imgIdx;

    
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &ctx->submitSemaphore;

    VK_CHECK(vkQueuePresentKHR(ctx->graphicsQueue, &presentInfo));
    VK_CHECK(vkDeviceWaitIdle(ctx->logicalDevice));

    vkFreeCommandBuffers(ctx->logicalDevice, ctx->commandPool, 1, &cmd);
}

/*
*@ initGLFW:
* initializes the glfw for vulkan support;
*/

void initGlfw() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW!");
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

int main() {
    initGlfw();

    Context ctx{};
    ctx.window = glfwCreateWindow(WIDTH, HEIGHT, "Window", 0, 0);

    createInstance(&ctx);
    createSurface(&ctx);
    pickGPU(&ctx);
    getQueueIndex(&ctx);
    createLogicalDevice(&ctx);
    createSwapchain(&ctx);
    createCommandPool(&ctx);
    createSemaphore(&ctx);
    createRenderPass(&ctx);
    createImageViews(&ctx);
    createFrameBuffer(&ctx);
    createPipelineLayout(&ctx);
    createPipeline(&ctx);

    while (!glfwWindowShouldClose(ctx.window)) {
        render(&ctx);
        glfwPollEvents();
    }
}