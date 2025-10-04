
#include "include_core.h"

renderer* tmp_opengl = nullptr;
input* tmp_input = nullptr;


PROC render_init() -> void
{
    bool vulkan_ok = false;
    bool opengl_ok = false;

    switch (global->render_backend)
    {
        case e_render_backend::vulkan:
            vulkan_ok = vulkan_init(); break;
        case e_render_backend::opengl:
            // opengl_ok = opengl_init(); break;
            tmp_opengl = new renderer();
            tmp_input = new input( tmp_opengl->platform );
            void(0);

    }
}

PROC render_tick() -> void
{
    switch (global->render_backend)
    {
        case e_render_backend::vulkan:
            vulkan_tick(); break;
        case e_render_backend::opengl:
            tmp_input->frame_update( 0 );
            tmp_opengl->frame_update();
            void(0);
    }
}
