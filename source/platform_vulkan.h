
#pragma once

struct vulkan_context
{
    // Primary Vulkan instance of to interface with
    VkInstance instance;
    // Primary Window surface to draw to
    VkSurfaceKHR surface;
};

PROC vulkan_allocator_create_callbacks( i_allocator* allocator );

PROC vulkan_init() -> fresult;

extern vulkan_context* g_vulkan;
