
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

fresult vulkan_init()
{
    fresult result = true;
    g_vulkan = memory_allocate<vulkan_context>( 1 );
    VkInstanceCreateInfo instance_args = {};
    VkApplicationInfo app_info = {};
    VkXlibSurfaceCreateInfoKHR surface_args = {};

    // Enumerate intance layers
    u32 n_layers = 0;
    vkEnumerateInstanceLayerProperties( &n_layers, nullptr );

    array<VkLayerProperties> layers;
    layers.change_allocation( n_layers );
    VkResult enumerate_layer_ok = vkEnumerateInstanceLayerProperties( &n_layers, layers.data );

    // Setup extensions and layers
    array<cstring> enabled_layers = {
        "VK_LAYER_KHRONOS_validation", // debug validaiton layer
    };
    array<cstring> enabled_extensions = {
        "VK_KHR_surface", // Surface for common window and compositing tasks
        "VK_KHR_xlib_surface" // xlib windowing extension
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

    if (g_x11->server && g_x11->window)
    {
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
    }
    else
    { vulkan_log( "Vulkan surface could not be created for platform Xlib" ); }

    vulkan_log( "Tachyon Vulkan", "Vulkan Initialized" );
    return true;
}
