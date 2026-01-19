
#pragma once

#define VMEC_SDL_ON 1
#if VMEC_SDL_ON

namespace tyon
{
    // Platform Hooks
    PROC sdl_init() -> fresult;
    PROC sdl_tick() -> fresult;
    PROC sdl_destroy() -> fresult;
    PROC sdl_window_open() -> fresult;
    PROC sdl_window_close() -> fresult;

    // Internal
    PROC sdl_event_process() -> void;

    PROC sdl_platform_procs_create() -> platform_procs;
}

#endif // VMEC_SDL_ON
