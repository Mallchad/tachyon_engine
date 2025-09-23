
#include "include_core.h"

vulkan_context* g_vulkan = nullptr;

#define vulkan_log( ... ) log( "Tachyon Vulkan", __VA_ARGS__ );
#define vulkan_error( ... ) TYON_LOG_ERROR( "Tachyon Vulkan", __VA_ARGS__ );
#define vulkan_logf( FORMAT_, ... ) log_format( "Tachyon Vulkan", (FORMAT_), __VA_ARGS__);
#define vulkan_errorf( FORMAT_, ... ) log_format_error( "Tachyon Vulkan", (FORMAT_),  __VA_ARGS__ );

PROC vulkan_allocator_create_callbacks( i_allocator* allocator )
{
    VkAllocationCallbacks result = {};

    TYON_TODO( "Unimplimented" );
    return result;
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
    };
    app_info.pApplicationName = "Tachyon Engine";
    app_info.applicationVersion = VK_MAKE_API_VERSION( 0, 0, 1, 0 );
    app_info.pEngineName = "Tachyon Engine";
    app_info.engineVersion = VK_MAKE_API_VERSION( 0, 0, 1, 0 );
    app_info.apiVersion = 0;
    instance_args.pApplicationInfo = &app_info;
    instance_args.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_args.enabledLayerCount = enabled_layers.size();
    instance_args.ppEnabledLayerNames = enabled_layers.data;
    instance_args.enabledExtensionCount = enabled_extensions.size();
    instance_args.ppEnabledExtensionNames = enabled_extensions.data;

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

    // Set all features to false for now
    VkPhysicalDeviceFeatures device_features = {};
    // Setup the final logical device args struct
    VkDeviceCreateInfo device_args = {};
    device_args.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_args.pQueueCreateInfos = queues.data;
    device_args.queueCreateInfoCount = queues.size();
    device_args.pEnabledFeatures = &device_features;

    array<cstring> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, // Presentation swapchain extension
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


    vulkan_log( "Tachyon Vulkan", "Vulkan Initialized" );
    TYON_BREAK();
    return true;
}

PROC vulkan_destroy() -> void
{
    g_vulkan->~vulkan_context();

}
