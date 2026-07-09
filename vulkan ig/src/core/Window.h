#ifndef WINDOW_H
#define WINDOW_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cstdio>

struct Window {
	const char* Title;
	uint32_t Width;
	uint32_t Height;
	GLFWwindow* Handle;
	bool Resized;
};

void init();
GLFWwindow* createWindow(Window* window);
GLFWwindow* initWindow(Window* window);
void destroyWindow(GLFWwindow* window);

#endif // WINDOW_H

#ifdef WINDOW_IMPLEMENTATION
#undef WINDOW_IMPLEMENTATION

GLFWwindow* initWindow(Window* window)
{
	init();
	GLFWwindow* win = createWindow(window);
	return win;
}

void init()
{
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW!");
		return;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void framebufferResizeCallback(GLFWwindow* win, int width, int height)
{
	Window* window = (Window*)glfwGetWindowUserPointer(win);
	window->Width = width;
	window->Height = height;
	window->Resized = true;
}

GLFWwindow* createWindow(Window* window)
{
	GLFWwindow* win = glfwCreateWindow(window->Width, window->Height, window->Title, nullptr, nullptr);
	if (!win) {
		fprintf(stderr, "[ERROR]: Could not Create Window");
		return nullptr;
	}

	window->Handle = win;
	glfwSetWindowUserPointer(win, window);
	glfwSetFramebufferSizeCallback(win, framebufferResizeCallback);

	return win;
}

void destroyWindow(GLFWwindow* window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

#endif // WINDOW_IMPLEMENTATION
