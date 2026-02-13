
#pragma

namespace tyon
{
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
