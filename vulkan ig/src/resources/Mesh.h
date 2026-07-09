#ifndef MESH_H
#define MESH_H

#include "../core/Buffers.h"
#include "../core/Device.h"

struct Vertex
{
	float pos[3];
	float color[3];
};

struct Mesh
{
	Device* Device;
	Buffer VertexBuffer;
	Buffer Indices;
	u32 IndicesCount;
	u32 VertexCount;
};

void createMesh(Mesh* mesh, CommandPool* commandPool, const std::vector<Vertex>& vertices);
void createMesh(Mesh* mesh, CommandPool* commandPool, const std::vector<Vertex>& cubeVerts, const std::vector<u32>& cubeIndices);

void destroyMesh(Mesh* mesh);


#endif // !MESH_H

#ifdef  MESH_IMPLEMENTATION
#undef  MESH_IMPLEMENTATION

void createMesh(Mesh* mesh, CommandPool* commandPool, const std::vector<Vertex>& vertices)
{
	mesh->VertexCount = (u32)vertices.size();
	VkDeviceSize size = sizeof(Vertex) * vertices.size();

	Buffer staging{};
	createBuffer(&staging, mesh->Device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* data;

	VK_CHECK(vkMapMemory(mesh->Device->LogicalDevice, staging.bufferMemory, 0, size, 0, &data));
	memcpy(data, vertices.data(), (size_t)size);
	vkUnmapMemory(mesh->Device->LogicalDevice, staging.bufferMemory);

	createBuffer(&mesh->VertexBuffer, mesh->Device, size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	copyBuffer(mesh->Device, commandPool, &staging, &mesh->VertexBuffer, size);
	destroyBuffer(mesh->Device, &staging);
}

void createMesh(Mesh* mesh, CommandPool* commandPool, const std::vector<Vertex>& cubeVerts, const std::vector<u32>& cubeIndices)
{
	mesh->VertexCount = (u32)cubeVerts.size();
	VkDeviceSize size = sizeof(Vertex) * cubeVerts.size();

	Buffer staging{};
	createBuffer(&staging, mesh->Device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* data;
	
	VK_CHECK(vkMapMemory(mesh->Device->LogicalDevice, staging.bufferMemory, 0, size, 0, &data));
	memcpy(data, cubeVerts.data(), (size_t)size);
	vkUnmapMemory(mesh->Device->LogicalDevice, staging.bufferMemory);
	
	createBuffer(&mesh->VertexBuffer, mesh->Device, size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	copyBuffer(mesh->Device, commandPool, &staging, &mesh->VertexBuffer, size);
	destroyBuffer(mesh->Device, &staging);

	mesh->IndicesCount = (u32)cubeIndices.size();
	VkDeviceSize size1 = sizeof(u32) * cubeIndices.size();

	Buffer staging1{};
	createBuffer(&staging1, mesh->Device, size1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* data1;

	VK_CHECK(vkMapMemory(mesh->Device->LogicalDevice, staging1.bufferMemory, 0, size1, 0, &data1));
	memcpy(data1, cubeIndices.data(), (size_t)size1);
	vkUnmapMemory(mesh->Device->LogicalDevice, staging1.bufferMemory);

	createBuffer(&mesh->Indices, mesh->Device, size1,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	copyBuffer(mesh->Device, commandPool, &staging1, &mesh->Indices, size1);
	destroyBuffer(mesh->Device, &staging1);
}

void destroyMesh(Mesh* mesh)
{
	destroyBuffer(mesh->Device, &mesh->VertexBuffer);
	destroyBuffer(mesh->Device, &mesh->Indices);
}

#endif // ! MESH_IMPLEMENTATION