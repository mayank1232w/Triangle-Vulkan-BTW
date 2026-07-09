#define WINDOW_IMPLEMENTATION
#include "core/Window.h"
#define INSTANCE_IMPLEMENTATION
#include "core/Instance.h"
#define DEVICE_IMPLEMENTATION
#include "core/Device.h"
#define SWAPCHAIN_IMPLEMENTATION
#include "core/Swapchain.h"
#define COMMANDPOOL_IMPLEMENTATION
#include "core/CommandPool.h"
#define FRAMEBUFFER_IMPLEMENTATION
#include "core/Framebuffer.h"
#define RENDERPASS_IMPLEMENTATION
#include "core/RenderPass.h"
#define SHADER_IMPLEMENTATION
#include "core/Shader.h"
#define SYNC_IMPLEMENTATION
#include"core/Sync.h"
#define PIPELINE_IMPLEMENTATION
#include "pipeline/Pipeline.h"
#define RENDERER_IMPLEMENTATION
#include "renderer/Renderer.h"
#define BUFFERS_IMPLEMENTATION
#include "core/Buffers.h"
#define MESH_IMPLEMENTATION
#include "resources/Mesh.h"
#define DEPTH_IMPLEMENTATION
#include "resources/Depth.h"

int main()
{
	Window win{};
	win.Width = 800;
	win.Height = 600;
	win.Title = "Window";

	GLFWwindow* window = initWindow(&win);
	
	Instance instance{};
	instance.enableDebug = true;
	
	initInstance(&instance);

	Device device{};
	initDevice(&device, instance.Instance, window);

	Swapchain swapchain{};
	swapchain.Device = &device;

	createSwapchain(&swapchain);
	createImageViews(&swapchain);

	Depth depthBuffer{};
	createDepthBuffer(&depthBuffer, &device, swapchain.Extent);   

	CommandPool commandPool{};
	commandPool.Device = &device;

	createCommandPool(&commandPool);

	RenderPass renderPass{};
	renderPass.Device = &device;
	renderPass.Swapchain = &swapchain;

	createRenderPass(&renderPass);

	Framebuffer framebuffer{};
	framebuffer.Device = &device;
	framebuffer.RenderPass = &renderPass;
	framebuffer.SwapChain = &swapchain;
	framebuffer.Window = &win;
	framebuffer.Depth = &depthBuffer;

	createFrameBuffer(&framebuffer);

	Shader shader{};
	shader.Device = &device;

	createShaderModule(&shader, "assets/shaders/shader.vert.spv", "assets/shaders/shader.frag.spv");

	Sync sync{};
	sync.Device = &device;

	createSemaphore(&sync);

	std::vector<Vertex> cubeVerts = {
		// pos                    color
		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // 0: left-bottom-back   (red)
		{{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, // 1: right-bottom-back  (green)
		{{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}}, // 2: right-top-back     (blue)
		{{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}}, // 3: left-top-back      (yellow)
		{{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}}, // 4: left-bottom-front  (magenta)
		{{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}}, // 5: right-bottom-front (cyan)
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}}, // 6: right-top-front    (white)
		{{-0.5f,  0.5f,  0.5f}, {0.2f, 0.2f, 0.2f}}, // 7: left-top-front     (dark gray)
	};

	std::vector<uint32_t> cubeIndices = {
		// back face   (-z), viewed from outside looking in +z direction
		0, 2, 1,   0, 3, 2,
		// front face  (+z), viewed from outside looking in -z direction
		4, 5, 6,   4, 6, 7,
		// left face   (-x)
		0, 4, 7,   0, 7, 3,
		// right face  (+x)
		1, 2, 6,   1, 6, 5,
		// bottom face (-y)
		0, 1, 5,   0, 5, 4,
		// top face    (+y)
		3, 7, 6,   3, 6, 2,
	};

	Mesh mesh{};
	mesh.Device = &device;
	createMesh(&mesh, &commandPool, cubeVerts, cubeIndices);
	
	Pipeline pipeline{};
	pipeline.Device = &device;
	pipeline.Shader = &shader;
	pipeline.RenderPass = &renderPass;

	createPipelineLayout(&pipeline);
	createPipeline(&pipeline);

	Renderer renderer{};
	renderer.Device = &device;
	renderer.CommandPool = &commandPool;
	renderer.Framebuffer = &framebuffer;
	renderer.Pipeline = &pipeline;
	renderer.RenderPass = &renderPass;
	renderer.Swapchain = &swapchain;
	renderer.Sync = &sync;

	renderer.Mesh = &mesh;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		render(&renderer);
	}
}