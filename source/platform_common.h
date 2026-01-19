
#pragma once

namespace tyon
{
    struct window
    {
        uid id;
        fstring name;
        fstring title;
        v2 size;
        v2 position;
        /** It's pretty rare the user wants a partial window, they can fix it
         * themselves if they want. */
        bool maximized = true;

    };

    struct platform_procs
    {
        typed_procedure<fresult()> init;
        typed_procedure<fresult()> tick;
        typed_procedure<fresult()> destroy;
        typed_procedure<fresult( window* arg )> window_open;
        typed_procedure<fresult( window* arg )> window_close;

        typed_procedure< fresult(window*, VkInstance,
            const struct VkAllocationCallbacks*, VkSurfaceKHR* ) > vulkan_surface_create;
    };
}
