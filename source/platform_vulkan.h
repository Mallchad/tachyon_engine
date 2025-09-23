
#pragma once

struct vulkan_context
{
    // Primary Vulkan instance of to interface with
    VkInstance instance;
    // Primary device to do work on
    VkPhysicalDevice device;
    // Logical device which is reserved logical resoruces from physical device
    VkDevice logical_device;
    // Primary Window surface to draw to
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    // Top level resource manager for vulkan
    resource_arena resources;
};

PROC vulkan_allocator_create_callbacks( i_allocator* allocator );

PROC vulkan_init() -> fresult;

PROC vulkan_destroy() -> void;

extern vulkan_context* g_vulkan;
