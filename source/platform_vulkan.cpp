
#include "include_core.h"

vulkan_context* g_vulkan = nullptr;

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
    VkXlibSurfaceCreateInfoKHR surface_args = {};

    if (g_x11->server && g_x11->window)
    {
        surface_args.dpy = g_x11->server;
        surface_args.flags = 0x0;
        surface_args.window = g_x11->window;

        tyon_log( "Creating Vulkan/Xlib drawing surface" );
        VkResult surface_ok = vkCreateXlibSurfaceKHR(
            g_vulkan->instance,
            &surface_args,
            nullptr,
            &g_vulkan->surface
        );
        ERROR_GUARD( surface_ok == VK_SUCCESS, "Vulkan/xlib Surface creation error" );
    }
    else
    { tyon_log( "Vulkan surface could not be created for platform Xlib" ); }

    return result;
}
