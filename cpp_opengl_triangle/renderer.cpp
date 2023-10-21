
#include "renderer.h"

// #define GL_GLEXT_PROTOTYPES
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <cmath>

#include <iostream>

CONSTRUCTOR renderer::renderer()
{
    std::cout << "Starting renderer\n" ;

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

bool renderer::draw_test_triangle()
{
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

    unsigned int vbo = -1;
    glGenBuffers( 1, &vbo);
    glBindBuffer( GL_ARRAY_BUFFER, vbo);
    glBufferData( GL_ARRAY_BUFFER, sizeof( mtest_triangle ), mtest_triangle, GL_STATIC_DRAW);

    const char* shader_vertex_source = "#version 330 core \n"
        "layout (location = 0) in vec3 pix; \n"
        "void main() \n"
        "{ \n"
        "    gl_Position = vec4(pix.x, pix.y, pix.z, 1.f); \n"
        "}";
    unsigned int shader_vertex = -1;
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
    unsigned int shader_fragment = -1;
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

    unsigned int shader_program = -1;
    shader_program = glCreateProgram();

    // Square Rendering
    GLfloat square_width = 200;
    GLfloat square_height = 200;
// (top-right corner anchored)
    GLfloat square_x = (1920/2) - (square_width/2);
    GLfloat square_y = (1080/2) - (square_height/2) + 200.f;

    for (int y = square_y; y < square_y + square_height; ++y)
    {
        for (int x = square_x; x < square_x+square_width; ++x)
        {
            if ( x + (1920*y) > (1920*1080) ) break;
            mbuffer[x + (1920*y)] = mtest_rectangle_color;
        }
    }

    return true;
}

bool renderer::draw_test_circle()
{
    // (center anchored)
    GLfloat circle_x = 1920/2;
    GLfloat circle_y = 1080/2 - 200;
    GLfloat circle_radius = 200.f;
    GLfloat circle_radius_squared = circle_radius * circle_radius;
    GLfloat circle_feather = 0.2f;          // percentage

    GLfloat distance = 0;
    for (int y = 0; y < 1080; ++y)
    {
        for (int x = 0; x < 1920; ++x)
        {
            distance = std::abs( (x-circle_x ) * ( x-circle_x)) + std::abs( (y - circle_y) * (y - circle_y));
            // distance = std::abs( (x*x ) - ( circle_x*circle_x)) + std::abs( (y*y ) - ( circle_y*circle_y)); // Parallelagram
            // distance = (x * x) + (y + y);
            if (distance < circle_radius_squared)
            {
                // Row Major
                mbuffer[x + (1920*y)] = circle_color;
            }
            else if (distance <
                     (circle_radius_squared * (1+circle_feather)) + (circle_radius * (1+circle_feather)) )
{
    // mbuffer[x + (1920*y)] = circle_color * ( 1/((sqrt(distance) - circle_radius) * (circle_feather)));
        mbuffer[x + (1920*y)] = mtest_rectangle_color;
}
        }
    }
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
        // glRasterPos2f(.5f, .5f); // dunno what this does
        glDrawPixels(1920, 1080, GL_RGBA, GL_FLOAT, mbuffer.get());
        glXSwapBuffers ( rx_display, vx_window );
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
