
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

    /* NOTE: [2026-02-13 Fri 17:05] Maybe instead of trying to randomally call
       single functions it would be better to register callbacks for this
       event. I have devised this platform_event structure of all possible
       platform_specific functions. Crude, but should work. It would also be
       more performant than blind message passing.

       NOTE: [2026-02-13 Fri 21:05] I don't really think this is a good idea, it
       complicats managements of platform layers / subsystems which leads to
       duplicate of code and boilerplate and complicated when trying bind and
       unbind events. Instead I should go back to the unified 'platform_procs'
       interface to added onto a list and use that as an "event driver", then it
       can be removed or disabled at will. I'm modify it slightly to carry some
       context with it. */
     struct platform_subsystem
     {
        fstring name;
        uid id;
        bool active = false;
        array<fstring> subsystem_dependencies;
        // TODO: Not sure how to fill this pointer right now.
        void* context = nullptr;

        typed_procedure<fresult()> init;
        typed_procedure<fresult()> tick;
        typed_procedure<fresult()> destroy;
        typed_procedure<fresult( window* arg )> window_open;
        typed_procedure<fresult( window* arg )> window_close;

        typed_procedure< fresult(window*, VkInstance,
            const struct VkAllocationCallbacks*, VkSurfaceKHR* ) > vulkan_surface_create;
    };

    struct platform_context
    {
        array<platform_subsystem> subsystems;
    };
    extern platform_context* g_platform;

    // TODO: Temporary
    FORWARD PROC sdl_create_platform_subsystem() -> platform_subsystem;

    template<>
    struct entity_type_definition<window>
    {
        using t_entity = window;
        using t_context = entity_type_context<t_entity>;
        static constexpr cstring name = "tyon::window";
        static constexpr u128 id = uuid("965fd378-6573-4844-aa34-e6645cc3ac7a");

        PROC allocate() -> void
        {}
        PROC init( t_entity* arg ) -> fresult
        {
            return false;
        }
        PROC destroy( t_entity* arg ) -> void
        {
            auto sdl = sdl_create_platform_subsystem();
            sdl.window_close( arg );
            *arg = {};
        }

        PROC tick() -> void
        {}

        static PROC context_tick( void* context ) -> void
        {}

        static PROC destroy_all( void* context ) -> void
        {}

    };

}
