#ifndef SHADER_H
#define SHADER_H

#include "Utils.h"
#include "Device.h"

struct Shader {
    Device* Device;
	VkShaderModule VertexShader;
	VkShaderModule FragmentShader;
};

void createShaderModule(Shader* shader, std::string vertexShader, std::string fragmentShader);
std::vector<char> readFile(const std::string& fileName);
void destroyShaderModule(Shader* shader);

#endif // !SHADER_H

#ifdef SHADER_IMPLEMENTATION
#undef SHADER_IMPLEMENTATION

std::vector<char> readFile(const std::string& fileName)
{
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open SPIR-V file: " + fileName);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void createShaderModule(Shader* shader, std::string vertexShader, std::string fragmentShader)
{

    std::vector<char> vertexCode = readFile(vertexShader);
    std::vector<char> fragmentCode = readFile(fragmentShader);

    VkShaderModuleCreateInfo vertexShaderInfo{};
    vertexShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertexShaderInfo.pCode = reinterpret_cast<const uint32_t*>(vertexCode.data());
    vertexShaderInfo.codeSize = vertexCode.size();

    VkShaderModuleCreateInfo fragmentShaderInfo{};
    fragmentShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragmentShaderInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentCode.data());
    fragmentShaderInfo.codeSize = fragmentCode.size();

    VK_CHECK(vkCreateShaderModule(shader->Device->LogicalDevice, &vertexShaderInfo, nullptr, &shader->VertexShader));
    VK_CHECK(vkCreateShaderModule(shader->Device->LogicalDevice, &fragmentShaderInfo, nullptr, &shader->FragmentShader));
}

void destroyShaderModule(Shader* shader) {
    vkDestroyShaderModule(shader->Device->LogicalDevice, shader->VertexShader, nullptr);
    vkDestroyShaderModule(shader->Device->LogicalDevice, shader->FragmentShader, nullptr);
}

#endif // !SHADER_IMPLEMENTATION