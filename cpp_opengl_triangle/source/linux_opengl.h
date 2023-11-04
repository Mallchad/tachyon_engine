
#pragma once

#include "renderer_interface.hpp"

#include <X11/Xlib.h>
#include <GL/glx.h>

#include <bits/unique_ptr.h>
#include <vector>

#include "math.hpp"
#include "global.h"

using std::unique_ptr;
using std::make_unique;

class renderer_opengl;
typedef renderer_opengl renderer_backend;

class renderer_opengl final
{
    global_database* global = nullptr;

    Display* rx_display = nullptr;
    Window vx_window = {};
    fint32 vx_default_screen = -1;
    fint32 display_width = -1;
    fint32 display_height = -1;
    int vx_connection_number = -1;
    char vx_connection_string[20] = {};

    XVisualInfo* vx_buffer_config = nullptr;
    XSetWindowAttributes vx_window_attributes = {};
    std::vector<Atom> vx_window_protocols = {};
    Atom vx_wm_delete_window = 0;
    Atom wm_state = 0;
    Atom wm_state_fullscreen = 0;
    Atom wm_state_maximized_vert = 0;
    Atom wm_state_maximized_horz = 0;
    Atom wm_state_above = 0;
    Atom wm_allowed_actions = 0;

    // GL X extensions
    // Priamry Context
    GLXContext vglx_context = nullptr;
    fint32 vglx_context_id = 0;

    std::vector<GLXContext> vglx_context_list;
    GLXFBConfig* vglx_fbconfigurations = nullptr;
    GLXFBConfig vglx_fbselection = nullptr;
    const char* vglx_extensions_string = nullptr;
    int vglx_fb_count = -1;
    int vglx_major = -1;
    int bglx_minor = -1;

    unsigned int vbo_actives[10] = {};
    unsigned int vbo = 0;
    unsigned int vao[10] = {};

    const char* shader_fragment_source = "#version 330 core \n"
        "out vec4 frag_color; \n"
        "void main() \n"
        "{ \n"
        "    frag_color = vec4(1.f, 0.5f, 0.2f, 1.f); \n"
        "}\0";
    const char* shader_vertex_source = "#version 330 core \n"
        "layout (location = 0) in vec3 pix; \n"
        "void main() \n"
        "{ \n"
        "    gl_Position = vec4(pix.x, pix.y, pix.z, 1.f); \n"
        "}\0";
    unsigned int shader_vertex = 0;
    unsigned int shader_fragment = 0;
    unsigned int shader_program = 0;
    char shader_info_log[512] = {};
    char shader_fragment_log[512] = {};
    int shader_link_sucess = 0;

    unique_ptr<vfloat4[]> mbuffer = make_unique<vfloat4[]>( 1920*1080 );
    float mtest_triangle[9] =
    {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    };

    int progress_x = 0;
    int progress_y = 0;

public:
    vfloat4 mtriangle_color = { .2f, .9f, .2f, 1.f };
    vfloat4 mrectangle_color = {.5f, .1f, .5f, 1.f};
    vfloat4 mcircle_color = { .2f, .9f, .2f, 1.f };
    vfloat4 msignfield_color = {1.f/255*98.f, 1.f/255*42.f, 1.f/255*125.f, 1.f};
    vfloat4 gradient_approximation[10000] = {};
    int buffer_damage_size = 1920*1080;

    CONSTRUCTOR renderer_opengl();

    GLXContext
    FUNCTION get_gl_context() const;

    fint32
    FUNCTION create_context();
    
    bool
    FUNCTION draw_test_triangle(vfloat4 p_color);

    bool
    FUNCTION draw_test_circle(vfloat4 p_color);

    bool
    FUNCTION draw_test_rectangle(vfloat4 p_color);

    bool
    FUNCTION draw_test_signfield(vfloat4 p_color);

    bool
    FUNCTION refresh();

    DESTRUCTOR virtual ~renderer_opengl();
};

INTERFACE_IMPLEMENT_RENDERER(renderer_opengl)
