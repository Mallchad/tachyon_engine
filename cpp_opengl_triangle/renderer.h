#pragma once

#include <X11/Xlib.h>
#include <GL/glx.h>

#include <vector>

#include "math.h"

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
    std::vector<float3> mbuffer = {};
    float mtest_triangle[9] =
    {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    };
    XEvent vx_event_query = {};
    bool vx_window_closed = false;

public:
    CONSTRUCTOR renderer();
    GLXContext get_gl_context() const;
    bool draw_test_triangle();
    bool refresh();
    DESTRUCTOR virtual ~renderer();
};
