
namespace tyon
{

    // Platform Hooks
    PROC sdl_init() -> fresult
    {
        return false;
    }

    PROC sdl_tick() -> fresult
    {
        return false;
    }

    PROC sdl_destroy() -> fresult
    {
        return false;
    }

    PROC sdl_window_open() -> fresult
    {
        return false;
    }

    PROC sdl_window_close() -> fresult
    {
        return false;
    }

    // Internal
    PROC sdl_event_process() -> void
    {

    }

    PROC sdl_platform_procs_create() -> platform_procs
    {
        platform_procs result = {
            .init = sdl_init,
            .tick = sdl_tick,
            .destroy = sdl_destroy,
            .window_open = sdl_window_open,
            .window_close = sdl_window_close
        };
        return result;
    }


}
