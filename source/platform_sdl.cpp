
namespace tyon
{
    sdl_context* g_sdl = nullptr;

    // Platform Hooks
    PROC sdl_init() -> fresult
    {   TYON_LOG( "Initialization Start for Platform SDL" );
        g_sdl = memory_allocate<sdl_context>( 1 );
        // NOTE: SDL must not ever move threads
        SDL_SetLogPriorities( SDL_LOG_PRIORITY_TRACE );

        if (REFLECTION_PLATFORM_LINUX)
        {   fstring x11_env = std::getenv( "DISPLAY" );
            fstring wayland_env = std::getenv( "WAYLAND_DISPLAY" );

            if (g_render->renderdoc_attached || g_render->nsight_attached)
            {   SDL_SetHint( SDL_HINT_VIDEO_DRIVER, "x11" );
                g_render->window_platform = e_window_platform::x11;
            }
        }

        // TODO: Init more stuff here as you use more things
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD );

        i32 major = 4;
        i32 minor = 4;
        SDL_GL_GetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, &major );
        SDL_GL_GetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, &minor );

        // Enumerate video drivers
        array<fstring> video_drivers;
        TYON_LOG( "Enumerating SDL Video Drivers: " );
        for (i32 i=0; i<100; ++i)
        {
            cstring x_driver = SDL_GetVideoDriver( i );
            if (x_driver == nullptr) { break; }
            TYON_LOGF( "    {}", video_drivers.push_tail( x_driver ) );
        }

        fstring video_driver = SDL_GetCurrentVideoDriver();
        if (video_driver == "x11")
        {   g_render->window_platform = e_window_platform::x11;
        }
        else if (video_driver == "wayland")
        {   g_render->window_platform = e_window_platform::wayland;
        }
        else if (video_driver == "windows")
        {   g_render->window_platform = e_window_platform::windows;
        }

        TYON_LOGF( "Current SDL selected video: {}", video_driver );
        TYON_LOG( "Initialization Complete for Platform SDL" );
        return true;
    }

    PROC sdl_tick() -> fresult
    {
        sdl_event_process();
        return true;
    }

    PROC sdl_destroy() -> fresult
    {
        g_sdl->~sdl_context();
        return true;
    }

    PROC sdl_window_open( window* arg ) -> fresult
    {   PROFILE_SCOPE_FUNCTION();
        TYON_LOG( "Opening Window using SDL platform" );

        sdl_window platform_window;
        arg->id = uuid_generate();
        platform_window.id = arg->id;

        // TODO: Temporarily hardcoded to Vulkan window type
        platform_window.handle = SDL_CreateWindow(
            arg->title.c_str(),
            arg->size.x,
            arg->size.y,
            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE );

        if ( platform_window.handle == nullptr)
        {   TYON_ERROR( "Failed to open SDL window" );
            return false;
        }
        // TODO: Hardcoded main window
        g_sdl->main_window = arg;
        &g_sdl->windows.push_tail( platform_window );

        TYON_LOG( "Showing SDL Window" );
        SDL_ShowWindow( platform_window.handle );
        if (arg->maximized)
        {   SDL_MaximizeWindow( platform_window.handle );
            TYON_LOG( "Maximizing SDL window" );
        }
        i32 window_width = 0;
        i32 window_height = 0;
        SDL_GetWindowSize( platform_window.handle, &window_width, &window_height );
        TYON_LOGF( "Created window size, width: {} : height {}" , window_height, window_height );

        return true;
    }

    PROC sdl_window_close( window* arg ) -> fresult
    {
        return false;
    }

    // Internal
    PROC sdl_event_process() -> void
    {
        SDL_PumpEvents();
    }

    PROC sdl_vulkan_surface_create(
        window* arg,
        VkInstance vk_instance,
        const struct VkAllocationCallbacks* vk_allocator,
        VkSurfaceKHR* surface
    ) -> fresult
    {
        if (arg == nullptr)
        {   TYON_ERROR( "Window is nullptr" );
            return false;
        }
        auto search = g_sdl->windows.linear_search( [=]( sdl_window& x ) {
            return x.id == arg->id; } );
        if (search.match_found == false)
        {   return false;
        }

        auto platform_window = search.match;
        bool create_ok = SDL_Vulkan_CreateSurface(
            platform_window->handle, vk_instance, vk_allocator, surface );
        if (create_ok == false)
        {   TYON_ERROR( "Failed to create Vulkan surface using SDL Platform" );
            return false;
        }
        TYON_LOG( "Created Vulkan surface using SDL Platform" );
        return true;
    }

    PROC sdl_platform_procs_create() -> platform_procs
    {
        platform_procs result = {
            .init = sdl_init,
            .tick = sdl_tick,
            .destroy = sdl_destroy,
            .window_open = sdl_window_open,
            .window_close = sdl_window_close,
            .vulkan_surface_create = sdl_vulkan_surface_create
        };
        return result;
    }


}
