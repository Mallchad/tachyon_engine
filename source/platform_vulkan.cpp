
#include "include_core.h"

vulkan_context* g_vulkan = nullptr;

#define vulkan_log( ... ) log( "Tachyon Vulkan", __VA_ARGS__ );
#define vulkan_error( ... ) TYON_LOG_ERROR( "Tachyon Vulkan Error", __VA_ARGS__ );
#define vulkan_logf( FORMAT_, ... ) log_format( "Tachyon Vulkan Error", (FORMAT_), __VA_ARGS__);
#define vulkan_errorf( FORMAT_, ... ) log_error_format_impl( \
        "Tachyon Vulkan Error", fmt::format((FORMAT_),  __VA_ARGS__) );

VKAPI_ATTR VKAPI_CALL
PROC vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data ) -> VkBool32
{

    log_format( "Vulkan Validation", "{}", callback_data->pMessage );

    return VK_FALSE;
}

PROC vulkan_allocator_create_callbacks( i_allocator* allocator )
{
    VkAllocationCallbacks result = {};

    TYON_TODO( "Unimplimented" );
    return result;
}

PROC vulkan_shader_init( vulkan_shader* arg ) -> fresult
{
    ERROR_GUARD( arg->code_binary, "Code provided must be binary SPIR-V currently" );
    arg->code = file_load_binary( arg->code.filename );
    ERROR_GUARD( arg->code.file_loaded, "Failed to load file" );

    VkShaderModuleCreateInfo shader_args{};
    shader_args.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_args.codeSize = arg->code.memory.size;
    shader_args.pCode = arg->code.memory.data;

    VkShaderModule& module = arg->module;
    auto module_ok = vkCreateShaderModule(
        g_vulkan->logical_device, &shader_args, nullptr, &module );
    if (module_ok != VK_SUCCESS)
    {
        vulkan_errorf( "Failed to create shader module: {}", arg->name );
        return false;
    }
    g_vulkan->resources.push_cleanup( [=]{
        vulkan_log( "Dstroying shader module:", arg->name )
        vkDestroyShaderModule( g_vulkan->logical_device, module, nullptr ); } );

    VkPipelineShaderStageCreateInfo shader_stage_args{};
    shader_stage_args.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_args.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stage_args.module = module;
    shader_stage_args.pName = "main";
    vulkan_logf( "Created shader module: {}", arg->name );
    return true;
}

PROC vulkan_init() -> fresult
{
    fresult result = true;
    g_vulkan = memory_allocate<vulkan_context>( 1 );
    auto& instance = g_vulkan->instance;
    VkInstanceCreateInfo instance_args = {};
    VkApplicationInfo app_info = {};
    VkXlibSurfaceCreateInfoKHR surface_args = {};
    i32 selected_queue_family = -1;
    i32 present_queue_index = -1;

    defer_procedure _exit = [&result] {
       if (result)
       { vulkan_log( "Vulkan Initialized" ); }
       else
       {
           vulkan_error( "Vulkan initialization failure, cleaning up resources." );
           g_vulkan->resources.~resource_arena();
       }
    };


    // Enumerate intance layers
    u32 n_layers = 0;
    vkEnumerateInstanceLayerProperties( &n_layers, nullptr );

    array<VkLayerProperties> layers;
    layers.change_allocation( n_layers );
    VkResult enumerate_layer_ok = vkEnumerateInstanceLayerProperties( &n_layers, layers.data );

    // -- Setup extensions and layers --
    array<cstring> enabled_layers = {
        "VK_LAYER_KHRONOS_validation", // debug validaiton layer
    };
    array<cstring> enabled_extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME, // Surface for common window and compositing tasks
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME, // xlib windowing extension
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME, // message callback and debug stuff
    };
    app_info.pApplicationName = "Tachyon Engine";
    app_info.applicationVersion = VK_MAKE_API_VERSION( 0, 0, 1, 0 );
    app_info.pEngineName = "Tachyon Engine";
    app_info.engineVersion = VK_MAKE_API_VERSION( 0, 0, 1, 0 );
    app_info.apiVersion = 0;

    VkDebugUtilsMessengerCreateInfoEXT messenger_args {};
    messenger_args.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messenger_args.messageSeverity = ~u32(0);
    messenger_args.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messenger_args.pfnUserCallback = vulkan_debug_callback;

    instance_args.pApplicationInfo = &app_info;
    instance_args.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_args.enabledLayerCount = enabled_layers.size();
    instance_args.ppEnabledLayerNames = enabled_layers.data;
    instance_args.enabledExtensionCount = enabled_extensions.size();
    instance_args.ppEnabledExtensionNames = enabled_extensions.data;
    instance_args.pNext = &messenger_args;

    VkResult instance_ok = vkCreateInstance( &instance_args, nullptr, &g_vulkan->instance );
    if (instance_ok != VK_SUCCESS)
    {
        vulkan_error( "Failed to create Vulkan instance" );
        return false;
    }
    g_vulkan->resources.push_cleanup( []{
        vulkan_log( "Destroying Vulkan instance" );
        vkDestroyInstance( g_vulkan->instance, nullptr);
        g_vulkan->instance = VK_NULL_HANDLE;
    });

    // need to do this to get validation layer callbacks aparently?
    auto dyn_vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
    VkDebugUtilsMessengerEXT debug_messenger {};
    dyn_vkCreateDebugUtilsMessengerEXT(instance, &messenger_args, nullptr, &debug_messenger );

    // -- Setup default window surfaces --
    if (g_x11->server && g_x11->window)
    {
        // TODO: Need to chain a present struct to try scaling options
        surface_args.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        surface_args.dpy = g_x11->server;
        surface_args.flags = 0x0;
        surface_args.window = g_x11->window;

        vulkan_log( "Creating Vulkan/Xlib drawing surface" );
        VkResult surface_ok = vkCreateXlibSurfaceKHR(
            g_vulkan->instance,
            &surface_args,
            nullptr,
            &g_vulkan->surface
        );
        ERROR_GUARD( surface_ok == VK_SUCCESS, "Vulkan/xlib Surface creation error" );
        vulkan_log( "Tachyon Vulkan", "Vulkan Instance created" );
        g_vulkan->resources.push_cleanup( [] {
            vulkan_logf( "Destroying surface 0x{:x}", u64(g_vulkan->surface) );
            vkDestroySurfaceKHR( g_vulkan->instance, g_vulkan->surface, nullptr );
        });
    }
    else
    { vulkan_log( "Vulkan surface could not be created for platform Xlib" ); }

    // -- Device Enumeration and Selection --
    array<VkPhysicalDevice> devices;
    u32 n_devices = 0;
    vkEnumeratePhysicalDevices( instance, &n_devices, nullptr );
    devices.resize( n_devices );
    vkEnumeratePhysicalDevices( instance, &n_devices, devices.data );

    if (n_devices <= 0)
    {
        vulkan_error( "No physical devices found. Bailing Vulkan initialization" );
        return false;
    }
    for (i32 i=0; i < devices.size(); ++i)
    {
        VkPhysicalDevice x_device = devices[i];
        VkPhysicalDeviceProperties props;
        bool suitible = true;
        bool dedicated_graphics = false;
        vkGetPhysicalDeviceProperties( x_device, &props );
        vulkan_logf( "Enumerated physical device: {} | {:x}:{:x}",
                     props.deviceName, props.vendorID, props.deviceID );
        /* The driver version is literally trash. It's a vendor specific bitmask
         * so you could never hope to get it a coherent number without a complex
         * codepath figuring out which bitmask to use for each card and also
         * somehow finding all the different vendor bitmasks in history. */
        vulkan_logf(
            "    Vulkan API version: {}.{}.{}", VK_VERSION_MAJOR(props.apiVersion),
            VK_VERSION_MINOR(props.apiVersion), VK_VERSION_PATCH(props.apiVersion) );
        if (props.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            dedicated_graphics = true;
            vulkan_log( "    Device Type: Discrete GPU");
        }

        array<VkQueueFamilyProperties> families;
        u32 n_families = 0;
        vkGetPhysicalDeviceQueueFamilyProperties( x_device, &n_families, nullptr );
        families.resize( n_families );
        vkGetPhysicalDeviceQueueFamilyProperties( x_device, &n_families, families.data );
        vulkan_log( "Queue Family Count: ", n_families );

        /* Seriously... Why. you have to reference the queue family by an
         * arbitrary index. you get whilst looping through */
        i32 ideal_queue_family = -1;
        for (int i_queue=0; i_queue < families.size(); ++i_queue)
        {
            VkBool32 present_support = false;
            bool graphics_queue = (families[ i_queue ].queueFlags & VK_QUEUE_GRAPHICS_BIT);
            vkGetPhysicalDeviceSurfaceSupportKHR(
                x_device, i_queue, g_vulkan->surface, &present_support );
            if (graphics_queue && ideal_queue_family < 0)
            {   suitible &= true;
                ideal_queue_family = i_queue;
            }
            else if (present_support && present_queue_index < 0)
            {   suitible &= true;
                present_queue_index = i_queue;
            }
        }
        if (ideal_queue_family < 0)
        {   vulkan_error( "Failed to find suitible queue family for device"
                          "Vulkan initialized failed" );
            return false;
        }

        bool select_device =
            (suitible && (dedicated_graphics || g_vulkan->device == VK_NULL_HANDLE));
        if (select_device)
        {
            g_vulkan->device = x_device;
            // Set queue family index only when a new device is selected
            selected_queue_family = ideal_queue_family;
            vulkan_log( "Changed primary graphics device to", props.deviceName );
        }
        vulkan_log( "" );
    }
    if (g_vulkan->device == VK_NULL_HANDLE)
    {   vulkan_error( "Could not find a suitible graphics device for some reason."
                      "Vulkan initialization failed. " );
        return false;
    }

    // -- Create Device and Device Queue --

    // Create some 'queue arg' structs and then pass it to the 'logical device' creation struct
    VkQueue graphics_queue {};
    VkQueue present_queue {};
    f32 queue_priority = 1.0f;
    array<VkDeviceQueueCreateInfo> queues;

    VkDeviceQueueCreateInfo graphics_queue_args{};
    graphics_queue_args.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphics_queue_args.queueFamilyIndex = selected_queue_family;
    graphics_queue_args.queueCount = 1;
    graphics_queue_args.pQueuePriorities = &queue_priority;
    queues.push_tail( graphics_queue_args );

    VkDeviceQueueCreateInfo present_queue_args{};
    present_queue_args.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    present_queue_args.queueFamilyIndex = present_queue_index;
    present_queue_args.queueCount = 1;
    present_queue_args.pQueuePriorities = &queue_priority;
    queues.push_tail( present_queue_args );

    VkPhysicalDeviceFeatures device_features {
        // for multisampling support
        .sampleRateShading = true,
    };

    // Setup the final logical device args struct
    VkDeviceCreateInfo device_args = {};
    device_args.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_args.pQueueCreateInfos = queues.data;
    device_args.queueCreateInfoCount = queues.size();
    device_args.pEnabledFeatures = &device_features;

    array<cstring> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, // Presentation swapchain extension
        VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME,
    };
    device_args.ppEnabledLayerNames = enabled_layers.data;
    device_args.enabledLayerCount = enabled_layers.size();
    device_args.ppEnabledExtensionNames = device_extensions.data;
    device_args.enabledExtensionCount = device_extensions.size();

    // Actually create logical device
    auto device_ok = vkCreateDevice(
        g_vulkan->device,
        &device_args,
        nullptr,
        &g_vulkan->logical_device
    );
    if (device_ok) { tyon_error( "Device creation error" ); }

    g_vulkan->resources.push_cleanup( []{
        vulkan_logf( "Destroy Logical Device 0x{:x}", u64(g_vulkan->logical_device) );
        vkDestroyDevice( g_vulkan->logical_device, nullptr );
        g_vulkan->logical_device =  VK_NULL_HANDLE;
    } );
    vkGetDeviceQueue( g_vulkan->logical_device, selected_queue_family, 0, &graphics_queue );
    vkGetDeviceQueue( g_vulkan->logical_device, selected_queue_family, 0, &present_queue );


    /* We need to know the capabilities of the surface associated with the physical device
       So we retreive those capabilities */
    VkSurfaceCapabilitiesKHR surface_capabilities {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        g_vulkan->device, g_vulkan->surface, &surface_capabilities );

    // Also diagnostics for device formats
    array<VkSurfaceFormatKHR> surface_formats;
    u32 n_surfaces {};
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        g_vulkan->device, g_vulkan->surface, &n_surfaces, nullptr );
    surface_formats.resize( n_surfaces );
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        g_vulkan->device, g_vulkan->surface, &n_surfaces, surface_formats.data );

    // Create swapchan for presentation of images to windows
    VkSwapchainCreateInfoKHR swapchain_args {};
    swapchain_args.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_args.minImageCount = 2;
    swapchain_args.surface = g_vulkan->surface;
    swapchain_args.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    // The format must match the physical surface formats
    swapchain_args.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    swapchain_args.imageExtent = VkExtent2D { 1920, 1080 };
    swapchain_args.imageArrayLayers = 1; // More than 1 if a stereoscopic application
    swapchain_args.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_args.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_args.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    swapchain_args.queueFamilyIndexCount = 2;
    u32 family_indexes[] = { u32(selected_queue_family), u32(present_queue_index) };
    swapchain_args.pQueueFamilyIndices = family_indexes;
    swapchain_args.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_args.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;


    auto swapchain_ok = vkCreateSwapchainKHR(
        g_vulkan->logical_device,
        &swapchain_args,
        nullptr,
        &g_vulkan->swapchain
    );
    if (swapchain_ok != VK_SUCCESS)
    {
        vulkan_error( "Failed to initialize swapchain" );
        return false;
    }
    vulkan_log( "Initialized presentation swapchain" );
    g_vulkan->resources.push_cleanup( [] {
        vkDestroySwapchainKHR( g_vulkan->logical_device, g_vulkan->swapchain, nullptr );
        vulkan_log( "Destroy swapchain" );
    } );

    array<VkImage> swapchain_images {};
    u32 n_swapchain_images = 0;
    vkGetSwapchainImagesKHR( g_vulkan->logical_device, g_vulkan->swapchain,
                             &n_swapchain_images, nullptr);
    swapchain_images.resize( n_swapchain_images );
    vkGetSwapchainImagesKHR( g_vulkan->logical_device, g_vulkan->swapchain,
                             &n_swapchain_images, swapchain_images.data );

    VkFenceCreateInfo fence_args {};
    VkFence frame_begin_fence;
    VkFence frame_aquire_fence;
    VkFence frame_end_fence;
    fence_args.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence( g_vulkan->logical_device, &fence_args, nullptr, &frame_end_fence );
    vkCreateFence( g_vulkan->logical_device, &fence_args, nullptr, &frame_aquire_fence );
    // Signal the begin fence to prevent it hanging
    fence_args.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence( g_vulkan->logical_device, &fence_args, nullptr, &frame_begin_fence );

    // Set draw commands
    u32 image_index {};
    vkAcquireNextImageKHR(
        g_vulkan->logical_device,
        g_vulkan->swapchain,
        UINT64_MAX,
        VK_NULL_HANDLE,
        frame_aquire_fence,
        &image_index
    );
    vkWaitForFences( g_vulkan->logical_device, 1, &frame_aquire_fence, true, 16'666'666 );
    vkResetFences( g_vulkan->logical_device, 1, &frame_aquire_fence );

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_args {};
    pool_args.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_args.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_args.queueFamilyIndex = selected_queue_family;
    VkResult pool_ok = vkCreateCommandPool(
        g_vulkan->logical_device, &pool_args, nullptr, &command_pool );
    if (pool_ok)
    {
        vulkan_error( "Failed to create command pool" );
        return false;
    }

    // Create a command buffer
    VkCommandBufferAllocateInfo command_args{};
    command_args.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_args.commandPool = command_pool;
    command_args.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_args.commandBufferCount = 1;
    VkResult command_buffer_ok = vkAllocateCommandBuffers(
        g_vulkan->logical_device, &command_args, &g_vulkan->commands );

    // Start writing draw commands
    VkCommandBufferBeginInfo begin_args {};
    begin_args.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_args.flags = 0; // Optional
    begin_args.pInheritanceInfo = nullptr; // Optional
    VkResult command_ok = vkBeginCommandBuffer( g_vulkan->commands, &begin_args );

    // -- Create graphics pipeline --
    // Create shaders of pipeline
    vulkan_shader vertex_shader;
    vulkan_shader fragment_shader;

    vertex_shader.name = "test_triangle";
    vertex_shader.code.filename = "shaders/test_triangle.vert.spv";
    vertex_shader.code_binary = true;
    vertex_shader.stage_flag = VK_SHADER_STAGE_VERTEX_BIT;
    fragment_shader.name = "test_triangle";
    fragment_shader.code.filename = "shaders/test_triangle.frag.spv";
    fragment_shader.code_binary = true;
    fragment_shader.stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;

    vulkan_shader_init( &vertex_shader );
    vulkan_shader_init( &fragment_shader );

    vulkan_pipeline pipeline {};
    pipeline.shaders.push_tail( vertex_shader );
    pipeline.shaders.push_tail( fragment_shader );

    array<VkPipelineShaderStageCreateInfo> stages;
    stages.change_allocation( pipeline.shaders.size() );
    for (i64 i=0; i < pipeline.shaders.size(); ++i)
    {
        auto& x_shader = pipeline.shaders[i];
        stages.push_tail( VkPipelineShaderStageCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = x_shader.stage_flag,
                .module = x_shader.module,
                .pName = x_shader.entry_point.c_str()
        });
    }

    VkFormatProperties format_props {};
    vkGetPhysicalDeviceFormatProperties(
        g_vulkan->device, VK_FORMAT_B8G8R8A8_UNORM, &format_props );
    // Mandated by the spec when setting vertex bindings
    bool vertex_buffer_ok = format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;
    if (!vertex_buffer_ok)
    { vulkan_error( "Format feature vertex buffer not supported for specified format" ); }

    // Create a render pass, will be passed to pipeline
    // sub-pass first
    VkSubpassDescription sub_pass {};
    sub_pass.pColorAttachments = nullptr; // TODO: Add colour attachment
    sub_pass.colorAttachmentCount = 0;

    VkRenderPass render_pass {};
    VkRenderPassCreateInfo pass_args{};
    pass_args.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    pass_args.attachmentCount = 0; // TODO: Fill in attachments
    pass_args.pAttachments = nullptr;
    pass_args.subpassCount = 1;
    pass_args.pSubpasses = &sub_pass;

    auto pass_ok = vkCreateRenderPass(
        g_vulkan->logical_device, &pass_args, nullptr, &render_pass );

    VkVertexInputBindingDescription binding {};
    binding.binding = 0; // vertex attribute binding/slot. leave as 0
    binding.stride = 4 * 3;
    // Not sure what this is. think it means pulling from instance wide stuffs?
    binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    VkVertexInputAttributeDescription vertex_attributes {};
    vertex_attributes.location = 0; // shader specific binding location
    vertex_attributes.binding = 0;
    vertex_attributes.format = VK_FORMAT_B8G8R8A8_UNORM;


    // Mesh vertex input args
    VkPipelineVertexInputStateCreateInfo vertex_args {};
    vertex_args.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_args.pVertexBindingDescriptions =  &binding;
    vertex_args.vertexBindingDescriptionCount = 1;
    vertex_args.pVertexAttributeDescriptions = &vertex_attributes;
    vertex_args.vertexAttributeDescriptionCount = 1;

    // Mesh rendering settings
    VkPipelineInputAssemblyStateCreateInfo input_args {};
    input_args.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_args.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_args.primitiveRestartEnable = VK_FALSE;

    // Rasterizer settings
    VkPipelineRasterizationStateCreateInfo raster_args {};
    raster_args.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_args.polygonMode = VK_POLYGON_MODE_FILL;
    raster_args.lineWidth = 1.0f;
    raster_args.cullMode = VK_CULL_MODE_BACK_BIT;
    raster_args.frontFace = VK_FRONT_FACE_CLOCKWISE;
    raster_args.depthBiasEnable = VK_FALSE;
    raster_args.depthBiasConstantFactor = 0.0f;         // Optional
    raster_args.depthBiasClamp = 0.0f;                  // Optional
    raster_args.depthBiasSlopeFactor = 0.0f;            // Optional

    VkViewport viewport_config {
        // Upper left coordinates
        .x = 0,
        .y = 0,
        .width = 1920,
        .height = 1080,
        // Configurable viewport depth, can configurable but usually between 0 and 1
        .minDepth = 0.0,
        .maxDepth = 1.0
    };

    // Only render into a certain portion of the viewport with scissors
    VkRect2D scissor_config {
        VkOffset2D { 0, 0 },
        VkExtent2D { 1920, 1080 }
    };

    VkPipelineViewportStateCreateInfo viewport_args {};
    viewport_args.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_args.pViewports = &viewport_config;
    // viewportCount and scissorCount must be the same
    viewport_args.viewportCount = 1;
    viewport_args.pScissors = &scissor_config;
    viewport_args.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo multisample_args {};
    multisample_args.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_args.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
    multisample_args.sampleShadingEnable = true;
    multisample_args.minSampleShading = 1.0;
    // wut is this
    multisample_args.pSampleMask = nullptr;
    multisample_args.alphaToCoverageEnable = false;
    multisample_args.alphaToOneEnable = false;

    VkPipelineLayout layout {};
    VkDescriptorSetLayout descriptor_layout {};

    // Set pipeline bindings here
    VkDescriptorSetLayoutCreateInfo descriptor_layout_args {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = 0x0,
        .pBindings = nullptr,
        .bindingCount = 0,
    };

    VkPipelineLayoutCreateInfo layout_args {};
    layout_args.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_args.pSetLayouts = &descriptor_layout;
    layout_args.setLayoutCount = 1;
    layout_args.pushConstantRangeCount = 0;
    layout_args.pPushConstantRanges = nullptr;

    auto descriptor_layout_ok = vkCreateDescriptorSetLayout(
        g_vulkan->logical_device, &descriptor_layout_args, nullptr, &descriptor_layout );

    auto layout_ok = vkCreatePipelineLayout(
        g_vulkan->logical_device, &layout_args, nullptr, &layout );
    ERROR_GUARD( !descriptor_layout_ok && !layout_ok, "Failed to pipeline layout" );

    // Pipeline creation args
    VkGraphicsPipelineCreateInfo pipeline_args {};
    pipeline_args.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_args.pStages = stages.data;
    pipeline_args.stageCount = stages.size();
    // pipeline_args.pVertexInputState = &vertexInputInfo;
    pipeline_args.pInputAssemblyState = &input_args;
    pipeline_args.pViewportState = &viewport_args;
    pipeline_args.pRasterizationState = &raster_args;
    pipeline_args.pMultisampleState = &multisample_args;
    // pipeline_args.pDepthStencilState = nullptr; // Optional
    // pipeline_args.pColorBlendState = &colorBlending;
    // pipeline_args.pDynamicState = &dynamicState;
    pipeline_args.layout = layout;
    pipeline_args.renderPass = render_pass;
    pipeline_args.subpass = 0;
    // pipeline_args.basePipelineHandle = VK_NULL_HANDLE; // Optional
    // pipeline_args.basePipelineIndex = -1; // Optional

    // Provide pipeline cache here if relevant
    vkCreateGraphicsPipelines(
        g_vulkan->logical_device,
        VK_NULL_HANDLE,
        1,
        &pipeline_args,
        nullptr,
        &g_vulkan->pipeline );

    // what the fuck is even happening.
    array<VkImageView> swapchain_image_views;
    array<VkFramebuffer> swapchain_buffers;
    swapchain_image_views.resize( 10 );
    swapchain_buffers.resize( 10 );

    // Set render pass start information
    VkClearValue clear_value {};
    clear_value.color = {{ 0.0f, 0.0f, 0.0f, 1.0f }};
    VkRenderPassBeginInfo render_pass_args{};
    render_pass_args.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_args.renderPass = render_pass;
    render_pass_args.framebuffer = swapchain_buffers[0];
    render_pass_args.renderArea.offset = {0, 0};
    render_pass_args.renderArea.extent = VkExtent2D { 1920, 1080 };
    render_pass_args.clearValueCount = 1;
    render_pass_args.pClearValues = &clear_value;

    vkCmdBeginRenderPass( g_vulkan->commands, &render_pass_args, VK_SUBPASS_CONTENTS_INLINE );
    vkCmdBindPipeline(
        g_vulkan->commands, VK_PIPELINE_BIND_POINT_GRAPHICS, g_vulkan->pipeline );

    for (i32 i = 0; i < swapchain_image_views.size(); i++)
    {
    VkImageView attachments[] = {
        swapchain_image_views[i]
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = render_pass;
    framebufferInfo.attachmentCount = 0;
    framebufferInfo.pAttachments = nullptr;
    framebufferInfo.width = 1920;
    framebufferInfo.height = 1080;
    framebufferInfo.layers = 1;

    VkResult framebuffer_ok = vkCreateFramebuffer(
        g_vulkan->logical_device, &framebufferInfo, nullptr, &swapchain_buffers[i] );
    }

    // Finalize frame and submit all commands
    VkSubmitInfo submit_args {};
    submit_args.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkResult sync_ok = vkWaitForFences(
        g_vulkan->logical_device,
        1,
        &frame_begin_fence,
        true,
        16'666'666
    );
    vkResetFences( g_vulkan->logical_device, 1, &frame_begin_fence );
    if (sync_ok != VK_SUCCESS)
    { tyon_error( "Failed to wait on frame start fence for some reason" ); }
    vkQueueSubmit( graphics_queue, 1, &submit_args, frame_end_fence );

    TYON_BREAK();
    result = true;
    return result;
}

PROC vulkan_destroy() -> void
{
    g_vulkan->~vulkan_context();
}
