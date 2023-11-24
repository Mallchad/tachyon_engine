#pragma once

#include "linux_opengl.h"

#include "code_helpers.h"
#include "error.hpp"

class input_xlib final
{
    global_database* global = nullptr;
    renderer_opengl* m_renderer = nullptr;
    Display* m_display = nullptr;
    Window m_window = 0;

    Atom m_wm_delete_window = 0;

    fint32 m_events_received = 0;
    fint32 m_tmp_frame_count = 0;
public:
    CONSTRUCTOR input_xlib( renderer_opengl& display_handle );

    /// Needs to get the XServer handle from the renderer because it combines
    /// display and input contexts
    freport
    FUNCTION initialize( renderer_opengl& display_handle );

    freport
    FUNCTION frame_update( ffloat epoch_elapsed );
};
