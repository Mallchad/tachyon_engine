
#include "renderer.h"

// #define GL_GLEXT_PROTOTYPES
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#include <bits/this_thread_sleep.h>
#include <chrono>
#include <cmath>
#include <iostream>

#include "Tracy.hpp"

using namespace std::chrono_literals;

CONSTRUCTOR renderer::renderer()
{
    std::cout << "Starting renderer\n" ;

    GLfloat circle_x = 1920.f / 2.f;
    GLfloat circle_y = 1080.f / 2.f;
    GLfloat circle_radius = 200.f;
    GLfloat circle_radius_squared = circle_radius * circle_radius;
    GLfloat circle_feather = 0.02f;          // percentage
    GLfloat circle_feather_thickness = circle_radius * circle_feather;

    GLfloat distance = 0.f;
    GLfloat color_mult = 0.f;

    for (int i_distance = 0; i_distance < 10000; ++i_distance)
    {
        color_mult = (1.f / circle_radius) * static_cast<float>( i_distance );
        gradient_approximation[i_distance] = msignfield_color * color_mult;
    }

    // Get a matching FB config
    static int vglx_visual_attributes[] =
        {
            GLX_X_RENDERABLE    , True,
            GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
            GLX_RENDER_TYPE     , GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
            GLX_RED_SIZE        , 8,
            GLX_GREEN_SIZE      , 8,
            GLX_BLUE_SIZE       , 8,
            GLX_ALPHA_SIZE      , 8,
            GLX_DEPTH_SIZE      , 24,
            GLX_STENCIL_SIZE    , 8,
            GLX_DOUBLEBUFFER    , True,
            //GLX_SAMPLE_BUFFERS  , 1,
            //GLX_SAMPLES         , 4,
            None
        };
// Setup X11 Window and OpenGL Context
    rx_display = XOpenDisplay(nullptr);
    if (rx_display == nullptr)
    {
        std::cout << "Could not open X display" << std::endl;
        throw(1);
    }
    vglx_fbconfigurations = glXChooseFBConfig(rx_display, DefaultScreen(rx_display), vglx_visual_attributes, &vglx_fb_count);
    vglx_fbselection = vglx_fbconfigurations[4];
    vx_buffer_config = glXGetVisualFromFBConfig(rx_display, vglx_fbselection);
    vglx_context =  glXCreateContext(rx_display, vx_buffer_config, nullptr, 2);


    vx_window_attributes.colormap = XCreateColormap( rx_display,
                                                     RootWindow(rx_display,
                                                                vx_buffer_config->screen),
                                                     vx_buffer_config->visual, AllocNone );
    vx_window_attributes.background_pixmap = None ;
    vx_window_attributes.border_pixel      = 0;
    vx_window_attributes.event_mask        = StructureNotifyMask;
    vx_window = XCreateWindow( rx_display,
                               RootWindow( rx_display, vx_buffer_config->screen ),
                               500, 500, 500, 500, 0,
                               vx_buffer_config->depth,
                               InputOutput,
                               vx_buffer_config->visual,
                               CWBorderPixel|CWColormap|CWEventMask,
                               &vx_window_attributes );
    XMapWindow( rx_display, vx_window );
    glXMakeCurrent( rx_display, vx_window, vglx_context);

}

GLXContext renderer::get_gl_context() const
{
    return vglx_context;
}

bool renderer::draw_test_triangle(float4 color)
{
    color = mrectangle_color;
    auto glBindBuffer    = reinterpret_cast<PFNGLBINDBUFFERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glBindBuffer" )));
    auto glGenBuffers    = reinterpret_cast<PFNGLGENBUFFERSPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGenBufers" )));
    auto glBufferData    = reinterpret_cast<PFNGLBUFFERDATAPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glBufferData" )));
    auto glCreateShader  = reinterpret_cast<PFNGLCREATESHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glCreateShader" )));
    auto glShaderSource  = reinterpret_cast<PFNGLSHADERSOURCEPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glShaderSource" )));
    auto glCompileShader = reinterpret_cast<PFNGLCOMPILESHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glCompileShader" )));
    auto glGetShaderiv   = reinterpret_cast<PFNGLGETSHADERIVPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGetShaderiv" )));
    auto glGetShaderInfoLog = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGetShaderInfoLog" )));
    auto glCreateProgram = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glCreateProgram" )));

    auto glXGetFBConfigAttrib = reinterpret_cast<PFNGLXGETFBCONFIGATTRIBPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glXGetFBConfigAttrib" )));

    unsigned int vbo = 0;
    glGenBuffers( 1, &vbo);
    glBindBuffer( GL_ARRAY_BUFFER, vbo);
    glBufferData( GL_ARRAY_BUFFER, sizeof( mtest_triangle ), mtest_triangle, GL_STATIC_DRAW);

    const char* shader_vertex_source = "#version 330 core \n"
        "layout (location = 0) in vec3 pix; \n"
        "void main() \n"
        "{ \n"
        "    gl_Position = vec4(pix.x, pix.y, pix.z, 1.f); \n"
        "}";
    unsigned int shader_vertex = 0;
    shader_vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader_vertex, 1, &shader_vertex_source, nullptr);
    glCompileShader(shader_vertex);

    int shader_compiled = false;
    char shader_info_log[512] = {};
    glGetShaderiv(shader_vertex, GL_COMPILE_STATUS, &shader_compiled);
    glGetShaderInfoLog(shader_vertex, 512, nullptr, shader_info_log);
    if (shader_compiled == false)
    {
        std::cout << "Shader compilation failed, error message: " << shader_info_log << "\n";
        throw(1);
    }

    const char* shader_fragment_source = "#version 330 core \n"
        "out vec4 frag_color; \n"
        "void main() \n"
        "{ \n"
        "    frag_color = vec4(1.f, 0.5f, 0.2f, 1.f); \n"
        "}";
    unsigned int shader_fragment = 0;
    shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader_fragment, 1, &shader_fragment_source, nullptr);
    glCompileShader(shader_fragment);

    char shader_fragment_log[512] = {};
    glGetShaderiv(shader_fragment, GL_COMPILE_STATUS, &shader_compiled);
    glGetShaderInfoLog(shader_fragment, 512, nullptr, shader_fragment_log);
    if (shader_compiled == false)
    {
        std::cout << "Shader compilation failed, error message: " << shader_info_log << "\n"
                  << shader_fragment_log << "\n";
        throw(1);
    }

    unsigned int shader_program = 0;
    shader_program = glCreateProgram();

    // Square Rendering
    GLfloat square_width = 200;
    GLfloat square_height = 200;
// (top-right corner anchored)
    GLfloat square_x = (1920.f / 2.f) - (square_width/2.f);
    GLfloat square_y = (1080.f / 2.f) - (square_height/2.f) + 200.f;

    for (int y = float(square_y); y < square_y + square_height; ++y)
    {
        for (int x = float(square_x); x < square_x+square_width; ++x)
        {
            if ( x + (1920*y) > (1920*1080) ) break;
            mbuffer[x + (1920*y)] = mrectangle_color;
        }
    }

    return true;
}

bool renderer::draw_test_circle(float4 p_color)
{
    // (center anchored)
    GLfloat circle_x = 1920.f / 2.f;
    GLfloat circle_y = 1080.f / 2.f;
    GLfloat circle_radius = 200.f;
    GLfloat circle_radius_squared = circle_radius * circle_radius;
    GLfloat circle_feather = 0.03f;          // percentage
    GLfloat circle_feather_thickness = circle_radius * circle_feather;

    GLfloat distance = 0;
    GLfloat color_mult = 0.f;
    for (int y = 0; y < 1080; ++y)
    {
        for (int x = 0; x < 1920; ++x)
        {
            distance = std::abs( (x-circle_x ) * ( x-circle_x)) + std::abs( (y - circle_y) * (y - circle_y));
            if (distance < circle_radius_squared)
            {
                // Row Major
                mbuffer[x + (1920*y)] = p_color;
            }
            else if (distance <
                     (circle_radius_squared * (1+circle_feather)) + (circle_radius * (1+circle_feather)) )
            {
                color_mult = (1- (1 / circle_feather_thickness) * (sqrtf(distance) - circle_radius) );
                mbuffer[x + (1920*y)] = p_color * (color_mult > 1.f ? 1 : color_mult);
            }
        }
    }
    return true;

}

bool renderer::draw_test_rectangle(float4 p_color)
{
    return true;
}

bool renderer::draw_test_signfield(float4 p_color)
{
    if (buffer_damage_size <= 0)
    {
        return false;
    }
    // (center anchored)
    GLfloat circle_x = 1920.f / 2.f;
    GLfloat circle_y = 1080.f / 2.f;
    GLfloat circle_radius = 200.f;
    GLfloat circle_radius_squared = circle_radius * circle_radius;
    GLfloat circle_feather = 0.02f;          // percentage
    GLfloat circle_feather_thickness = circle_radius * circle_feather;

    GLfloat distance = 0;

    GLfloat color_mult = 1; // magic number

    for (int y = 0; y < 1080; ++y)
    {
        for (int x = 0; x < 1920; ++x)
        {
            distance = std::abs( (0.f +x-circle_x ) * ( 0.f +x-circle_x)) + std::abs( (y - circle_y) * (y - circle_y));

            color_mult = ((1 / circle_radius) * sqrtf(distance) ) * 0.8;
            mbuffer[x + (1920*y)] = p_color * color_mult;
        }
    }
    buffer_damage_size = 1920*1080;
    return true;

}

bool renderer::refresh()
{

    // Does nothing
    // XCheckTypedEvent(rx_display, DestroyNotify, &vx_event_query);
    // if (vx_window_closed) break;
    try {
        glClearColor( 0, 0.5, 1, 1 );
        glClear( GL_COLOR_BUFFER_BIT );
        glDrawPixels(1920, 1080, GL_RGBA, GL_FLOAT, mbuffer.get());
        glXSwapBuffers ( rx_display, vx_window );
        buffer_damage_size = 0;
    }
    catch (...) {}
    return true;
}

DESTRUCTOR renderer::~renderer()
{
// Cleanup
    std::cout << "Exiting Application" << std::endl;
    XFree(vx_buffer_config);
    XCloseDisplay(rx_display);
}
