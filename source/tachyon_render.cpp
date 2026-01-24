
#include "include_core.h"

namespace tyon
{

render_context* g_render = nullptr;

auto sdl = tyon::sdl_platform_procs_create();

PROC render_init() -> void
{
    PROFILE_SCOPE_FUNCTION();

    g_render = memory_allocate<render_context>( 1 );

    /* If nsight or renderdoc is attached it will break with wayland, so we
       should try to disable pre-emptively that if possible. */
    if (REFLECTION_PLATFORM_LINUX)
    {   char* renderdoc_env = std::getenv( "ENABLE_VULKAN_RENDERDOC_CAPTURE" );
        if (renderdoc_env && renderdoc_env[0] == '1')
        {   TYON_LOG( "Renderdoc is attached and doesn't support Wayland, falling back to X11" );
            g_render->renderdoc_attached = true;
        }
    }

    // SDL needs to setup after the render context but before vulkan init
    sdl.init();

    tyon::window default_window = {
        .name = "VMEC | Spectral Renderer",
        .size = tyon::v2 { 1920.0f, 1000.0f },
        .position = tyon::v2 { 0.0f, 0.0f },
        .maximized = true
    };
    sdl.window_open( &default_window );

    bool vulkan_ok = false;
    bool opengl_ok = false;

    switch (global->render_backend)
    {
        case e_render_backend::vulkan:
            x11_init();
            x11_window_open();
            vulkan_ok = vulkan_init();
            break;
        case e_render_backend::opengl:
            // opengl_ok = opengl_init(); break;
            TYON_BREAK();
            break;
        default: break;

    }
}

PROC render_tick() -> void
{
    PROFILE_SCOPE_FUNCTION();

    sdl.tick();
    switch (global->render_backend)
    {
        case e_render_backend::vulkan:
            vulkan_tick();
            break;
        case e_render_backend::opengl:
            TYON_BREAK();
            break;
        default: break;
    }
}

PROC mesh_init( mesh* arg ) -> fresult
{
    if (arg->id.valid())
    {   VULKAN_ERRORF( "Tried to initialize already initialize mesh '{}'", arg->name );
        return false;
    }
    if (arg->vertexes.size() <= 0)
    {   VULKAN_ERRORF( "Tried to initialize mesh '{}' with {} vertexes",
                       arg->name, arg->vertexes.size() * 3 );
        return false;
    }

    arg->id = uuid_generate();
    arg->faces_n = arg->vertexes.size() / 3;
    arg->vertexes_n = arg->vertexes.size();
    arg->vertex_indexes_n = arg->vertex_indexes.size();

    TYON_LOG( "Created mesh: ")
    TYON_LOGF( "    ID: {}", arg->id );
    TYON_LOGF( "    Name: {}", arg->name );
    TYON_LOGF( "    Faces: {}", arg->faces_n );
    TYON_LOGF( "    Vertexes: {}", arg->vertexes_n );
    TYON_LOGF( "    Vertex Indexes: {}", arg->vertex_indexes_n );
    return true;
}

}
