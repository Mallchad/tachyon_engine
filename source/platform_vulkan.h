

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
    /** Views describe how to interpret VkImage's, VkImages are related to
        textures and framebuffers */
    array<VkImageView> swapchain_image_views;
    array<VkFramebuffer> swapchain_framebuffers;
    array<VkImage> swapchain_images;
    VkQueue graphics_queue;
    VkQueue present_queue;

    VkRenderPass render_pass;
    // Primary graphics pipeline, associated with render pass
    VkPipeline pipeline;

    // Ungrouped threading primitives
    VkFence frame_begin_fence;
    VkFence frame_aquire_fence;
    VkFence frame_end_fence;

    VkSemaphore queue_submit_semaphore;
    VkSemaphore frame_end_semaphore;

    // Test Data
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

    // Top level resource manager for vulkan
    resource_arena resources;

    // Platform Independant State
    bool initialized = false;
    i64 frames_started = 0;
    i64 frames_completed = 0;
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

PROC vulkan_label_object( u64 handle, VkObjectType type, fstring name ) -> void;

PROC vulkan_init() -> fresult;

PROC vulkan_destroy() -> void;

PROC vulkan_tick() -> void;

PROC vulkan_draw() -> void;

extern vulkan_context* g_vulkan;
