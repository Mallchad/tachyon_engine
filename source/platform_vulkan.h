
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
    VkCommandBuffer commands;
    // Primary graphics pipeline
    VkPipeline pipeline;

    // Top level resource manager for vulkan
    resource_arena resources;
};

struct vulkan_shader
{
    fstring name = "unnamed";
    fstring entry_point = "main";
    file code;
    bool code_binary = false;
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage_flag {};
};

struct vulkan_pipeline
{
    fstring name = "unnamed";
    array<vulkan_shader> shaders;
};

PROC vulkan_allocator_create_callbacks( i_allocator* allocator );

PROC vulkan_init() -> fresult;

PROC vulkan_destroy() -> void;

extern vulkan_context* g_vulkan;
