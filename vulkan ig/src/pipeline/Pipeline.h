#ifndef PIPELINE_H
#define PIPELINE_H

#include "../core/Utils.h"
#include "../core/Device.h"
#include "../core/RenderPass.h"
#include "../core/Shader.h"
#include "../resources/Mesh.h"


struct Pipeline
{
    Device* Device;
    RenderPass* RenderPass;
    Shader* Shader;

    VkPipelineLayout PipelineLayout;
    VkPipeline Pipeline;
};

void createPipelineLayout(Pipeline* pipeline);
void createPipeline(Pipeline* pipeline);
void destroyPipeline(Pipeline* pipeline);

#endif // !PIPELINE_H

#ifdef PIPELINE_IMPLEMENTATION
#undef PIPELINE_IMPLEMENTATION

void createPipelineLayout(Pipeline* pipeline)
{
    VkPushConstantRange pushConstant{};

    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(glm::mat4);

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.pushConstantRangeCount = 1;
    createInfo.pPushConstantRanges = &pushConstant;

    VK_CHECK(vkCreatePipelineLayout(pipeline->Device->LogicalDevice, &createInfo, nullptr, &pipeline->PipelineLayout));
}

void createPipeline(Pipeline* pipeline)
{
    VkPipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineShaderStageCreateInfo vertexStage{};
    vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStage.pName = "main";
    vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStage.module = pipeline->Shader->VertexShader;

    VkPipelineShaderStageCreateInfo fragmentStage{};
    fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStage.pName = "main";
    fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStage.module = pipeline->Shader->FragmentShader;

    VkPipelineShaderStageCreateInfo shaderStage[] = {
        vertexStage,
        fragmentStage
    };

    VkVertexInputBindingDescription vertexBinding{};
    vertexBinding.binding = 0;
    vertexBinding.stride = sizeof(Vertex);
    vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertexAttrib[2]{};
    vertexAttrib[0].location = 0;
    vertexAttrib[0].binding = 0;
    vertexAttrib[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttrib[0].offset = offsetof(Vertex, pos);

    vertexAttrib[1].location = 1;
    vertexAttrib[1].binding = 0;
    vertexAttrib[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttrib[1].offset = offsetof(Vertex, color);

    VkPipelineVertexInputStateCreateInfo vertexInputState{};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexAttributeDescriptionCount = 2;
    vertexInputState.vertexBindingDescriptionCount   = 1;
    vertexInputState.pVertexAttributeDescriptions = vertexAttrib;
    vertexInputState.pVertexBindingDescriptions = &vertexBinding;

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

    VkPipelineDepthStencilStateCreateInfo depthStencil{};

    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    
    VkGraphicsPipelineCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount = ARRAY_SIZE(shaderStage);
    createInfo.pStages = shaderStage;
    createInfo.pVertexInputState = &vertexInputState;
    createInfo.renderPass = pipeline->RenderPass->RenderPass;
    createInfo.pColorBlendState = &colorBlendState;
    createInfo.pRasterizationState = &rasterizationState;
    createInfo.layout = pipeline->PipelineLayout;
    createInfo.pDynamicState = &dynamicStateInfo;
    createInfo.pViewportState = &viewportState;
    createInfo.pMultisampleState = &multisampleState;
    createInfo.pInputAssemblyState = &inputAssemblyState;
    createInfo.pDepthStencilState = &depthStencil;

    VK_CHECK(vkCreateGraphicsPipelines(pipeline->Device->LogicalDevice, NULL, 1, &createInfo, nullptr, &pipeline->Pipeline));
    destroyShaderModule(pipeline->Shader);
}

void destroyPipeline(Pipeline* pipeline)
{
    vkDestroyPipeline(pipeline->Device->LogicalDevice, pipeline->Pipeline, nullptr);
    vkDestroyPipelineLayout(pipeline->Device->LogicalDevice, pipeline->PipelineLayout, nullptr); 
}

#endif // !PIPELINE_IMPLEMENTATION
