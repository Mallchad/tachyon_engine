
#pragma once

struct vulkan_shader
{
    uid id;
    fstring name = "unnamed";
    fstring entry_point = "main";
    file code;
    bool code_binary = false;
    VkShaderModule platform_module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage_flag {};
};

struct vulkan_pipeline
{
    uid id;
    fstring name = "unnamed";
    array<vulkan_shader> shaders;
};

struct vulkan_swapchain
{
    uid id;
    fstring name = "unnamed";
    VkSwapchainKHR platform_swapchain;
    VkFramebuffer platform_framebuffer;
    array<VkFence> frame_end_fences;
    /** Vulkan dependant size of presentable surface, often close to window size.
    ultra pedantic about timing and exact size on most platforms. */
    VkExtent2D present_size;
    i32 n_images = 0;
    bool initialized = false;
    resource_arena resources;
};

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
    VkCommandPool command_pool;
    array<VkCommandBuffer> commands;
    /** Views describe how to interpret VkImage's, VkImages are related to
        textures and framebuffers */
    array<VkImageView> swapchain_image_views;
    array<VkFramebuffer> swapchain_framebuffers;
    array<VkImage> swapchain_images;
    i32 graphics_queue_family = -1;
    i32 present_queue_family = -1;
    VkQueue graphics_queue;
    VkQueue present_queue;

    VkDescriptorSetLayout descriptor_layout;
    VkPipelineLayout pipeline_layout;

    VkRenderPass render_pass;
    // Primary graphics pipeline, associated with render pass
    VkPipeline pipeline;

    // Ungrouped threading primitives
    VkFence frame_begin_fence;
    VkFence frame_acquire_fence;
    /* VkFence frame_end_fence; */

    VkSemaphore queue_submit_semaphore;
    VkSemaphore frame_end_semaphore;

    VkAllocationCallbacks allocator_callback;
    i32 frame_max_inflight = 2;
    vulkan_swapchain swapchain;

    // Test Data
    VkDeviceMemory vertex_memory;
    VkBuffer test_triangle_buffer {};
    array<f32> test_triangle_data = {
        0.f, 0.f, 1.f, // TODO: Remove temporary colour
        -0.5f, -0.4330127019f, 0.0f,
        0.f, 1.f, 0.f,
        0.5f, -0.4330127019f, 0.0f,
        1.f, 0.f, 0.f,
        0.0f,  0.4330127019f, 0.0f
    };


    // Configurables
    VkFormat swapchain_image_format = VK_FORMAT_B8G8R8A8_UNORM;


    // Platform Independant State
    /* memory_stack_allocator default_allocator; */
    std::unique_ptr<i_allocator> allocator = std::make_unique<memory_heap_allocator>();
    /* Top level resource manager for vulkan, needs to be ordered after allocator for
     because Vulkan resoruces are allocated out of the allocator*/
    resource_arena resources;

bool initialized = false;
    i64 frames_started = 0;
    i64 frames_completed = 0;
};

PROC vulkan_allocator_create_callbacks( i_allocator* allocator );

PROC vulkan_label_object( u64 handle, VkObjectType type, fstring name ) -> void;

PROC vulkan_swapchain_init( vulkan_swapchain* arg, VkSwapchainKHR reuse_swapchain )
    -> fresult;

PROC vulkan_init() -> fresult;

PROC vulkan_destroy() -> void;

PROC vulkan_tick() -> void;

PROC vulkan_draw() -> void;

extern vulkan_context* g_vulkan;
