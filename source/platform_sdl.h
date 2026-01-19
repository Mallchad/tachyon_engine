
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

    inline int rawr() { return 1;}
    inline void bar() {}

    inline void test()
    {

        typed_procedure<int()> foo = rawr;
        typed_procedure foo2 = rawr;
        /* typed_procedure foo3 = bar; */
    }

    struct sdl_procs
    {
        /* static constexpr typed_procedure init = &sdl_init; */
        /* static typed_procedure tick = &sdl_tick; */
        /* static typed_procedure destroy = &sdl_destroy; */
        /* static typed_procedure window_destroy = &sdl_window_open; */
        /* static typed_procedure window_open = &sdl_window_open; */
    };

}

#endif // VMEC_SDL_ON
