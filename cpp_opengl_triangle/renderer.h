#pragma once

#include <X11/Xlib.h>
#include <GL/glx.h>

#include <bits/unique_ptr.h>

#include "math.h"

using std::unique_ptr;
using std::make_unique;

class renderer
{
    Display* rx_display = nullptr;
    XVisualInfo* vx_buffer_config = nullptr;
    XSetWindowAttributes vx_window_attributes = {};
    Window vx_window = {};
    // GL X extensions
    GLXContext vglx_context = nullptr;
    GLXFBConfig* vglx_fbconfigurations = nullptr;
    GLXFBConfig vglx_fbselection = nullptr;
    int vglx_fb_count = -1;
    int vglx_major = -1;
    int bglx_minor = -1;
    unique_ptr<float4[]> mbuffer = make_unique<float4[]>( 1920*1080 );
    float mtest_triangle[9] =
    {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    };
    XEvent vx_event_query = {};
    bool vx_window_closed = false;

public:
    float4 mtest_rectangle_color = {.5f, .1f, .5f, 1.f};
    float4 circle_color = { .2f, .9f, .2f, 1.f };


    CONSTRUCTOR renderer();
    GLXContext get_gl_context() const;
    bool draw_test_triangle();
    bool draw_test_circle();
    bool refresh();
    DESTRUCTOR virtual ~renderer();
};
