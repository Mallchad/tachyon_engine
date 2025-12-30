
#include "include_core.h"

render_context* g_render = nullptr;
renderer* tmp_opengl = nullptr;
input* tmp_input = nullptr;


PROC render_init() -> void
{
    PROFILE_SCOPE_FUNCTION();

    g_render = memory_allocate<render_context>( 1 );

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
            tmp_opengl = new renderer();
            tmp_input = new input( tmp_opengl->platform );
            void(0);
            break;
        default: break;

    }
}

PROC render_tick() -> void
{
    PROFILE_SCOPE_FUNCTION();

    switch (global->render_backend)
    {
        case e_render_backend::vulkan:
            vulkan_tick();
            break;
        case e_render_backend::opengl:
            tmp_input->frame_update( 0 );
            tmp_opengl->frame_update();
            void(0);
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
    arg->faces_n = arg->vertexes.size();
    arg->vertexes_n = arg->vertexes.size() * 3;
    arg->vertex_indexes_n = arg->vertex_indexes.size();
    return true;
}
