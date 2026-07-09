#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>
#include <fstream>
#include <glm/mat4x4.hpp>


typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;


#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define VK_CHECK(result)                         \
    if (result != VK_SUCCESS)                    \
    {                                            \
        fprintf(stderr, "[ERROR]: %d", result);  \
        __debugbreak();                          \
    }

