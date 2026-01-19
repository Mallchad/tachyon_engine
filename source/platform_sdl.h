
#pragma once

#define VMEC_SDL_ON 1
#if VMEC_SDL_ON

#if (REFLECTION_COMPILER_MINGW)
    #define SDL_MAIN_HANDLED 1
#endif // REFLECTION_COMPILER_MINGW

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace tyon
{
    struct sdl_window
    {
        uid id;
        SDL_Window* handle;
    };

    struct sdl_context
    {
        window* main_window;
        array<sdl_window> windows;
    };

    // Platform Hooks
    PROC sdl_init() -> fresult;
    PROC sdl_tick() -> fresult;
    PROC sdl_destroy() -> fresult;
    PROC sdl_window_open( window* arg ) -> fresult;
    PROC sdl_window_close( window* arg ) -> fresult;

    // Internal
    PROC sdl_event_process() -> void;

    PROC sdl_platform_procs_create() -> platform_procs;

    extern sdl_context* g_sdl;
}

#endif // VMEC_SDL_ON
