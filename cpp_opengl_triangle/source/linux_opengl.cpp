#include "renderer_interface.hpp"
#include "linux_opengl.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#include <bits/this_thread_sleep.h>
#include <chrono>
#include <cmath>
#include <iostream>
#include <csignal>

#include "global.h"

#include "include_tracy.h"

using namespace std::chrono_literals;
using namespace std::string_literals;

typedef renderer_opengl def;

// Try to temrinate gracefully after attempt to kill X window
static void
sigterm_handler(int sig)
{
    (void)(sig);
    global_database* tmp = {};
    tmp = global_database::get_primary();
    tmp->kill_program = true;
    std::cout << "caught a signal\n" << std::flush;
    std::signal(SIGINT, sigterm_handler);
}
CONSTRUCTOR
def::renderer_opengl()
{
    this->initialize();
}

fhowdit
FUNCTION def::initialize()
{
    signal(SIGINT, sigterm_handler);
    global = global_database::get_primary();
    if (global == nullptr)
    {
        std::cout << "Could not aquire global database, cannot manage program safely \n";
        throw(1);

    }

    vglx_context_list.reserve(100);

    // -- GLX and X11 Initialization --
    ldynamic::glXCreateContextAttribsARB = reinterpret_cast<PFNGLXCREATECONTEXTATTRIBSARBPROC>(
       glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glXCreateContextAttribsARB" )));
    ldynamic::glXChooseFBConfig= reinterpret_cast<PFNGLXCHOOSEFBCONFIGPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glXChooseFBConfig" )));
    ldynamic::glXSwapIntervalMESA = reinterpret_cast<PFNGLXSWAPINTERVALMESAPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glXSwapIntervalMESA" )));
    ldynamic::glXSwapIntervalEXT = reinterpret_cast<PFNGLXSWAPINTERVALEXTPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glXSwapIntervalEXT" )));

    ldynamic::glBindBuffer        = reinterpret_cast<PFNGLBINDBUFFERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glBindBuffer" )));
    ldynamic::glGenBuffers        = reinterpret_cast<PFNGLGENBUFFERSPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGenBuffers" )));
    ldynamic::glBufferData        = reinterpret_cast<PFNGLBUFFERDATAPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glBufferData" )));
    ldynamic::glCreateShader      = reinterpret_cast<PFNGLCREATESHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glCreateShader" )));
    ldynamic::glShaderSource      = reinterpret_cast<PFNGLSHADERSOURCEPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glShaderSource" )));
    ldynamic::glCompileShader     = reinterpret_cast<PFNGLCOMPILESHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glCompileShader" )));

    ldynamic::glGetShaderiv       = reinterpret_cast<PFNGLGETSHADERIVPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGetShaderiv" )));
    ldynamic::glGetShaderInfoLog  = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGetShaderInfoLog" )));
    ldynamic::glCreateProgram     = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glCreateProgram" )));
    ldynamic::glAttachShader      = reinterpret_cast<PFNGLATTACHSHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glAttachShader" )));
    ldynamic::glDetachShader      =  reinterpret_cast<PFNGLATTACHSHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glDetachShader" )));
    ldynamic::glDeleteShader      = reinterpret_cast<PFNGLDELETESHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glDeleteShader" )));

    ldynamic::glLinkProgram       = reinterpret_cast<PFNGLLINKPROGRAMPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glLinkProgram" )));
    ldynamic::glGetProgramiv      = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGetProgramiv" )));
    ldynamic::glUseProgram = reinterpret_cast<PFNGLUSEPROGRAMPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glUseProgram" )));
    ldynamic::glGetProgramInfoLog = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGetProgramInfoLog" )));

    ldynamic::glVertexAttribPointer=reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glVertexAttribPointer" )));
    ldynamic::glGenVertexArrays = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGenVertexArrays" ) ));
    ldynamic::glEnableVertexAttribArray = reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glEnableVertexAttribArray" )));
    ldynamic::glEnableVertexArrayAttrib = reinterpret_cast<PFNGLENABLEVERTEXARRAYATTRIBPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glEnableVertexArrayAttrib" )));
    ldynamic::glBindVertexArray = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glBindVertexArray" )));
    ldynamic::glMapBuffer  = reinterpret_cast<PFNGLMAPBUFFERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glMapBuffer" )));
    ldynamic::glUnmapBuffer  = reinterpret_cast<PFNGLUNMAPBUFFERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glUnmapBuffer" )));

    ldynamic::glDrawArrays = reinterpret_cast<PFNGLDRAWARRAYSPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glDrawArrays" )));
    ldynamic::glDrawArraysEXT = reinterpret_cast<PFNGLDRAWARRAYSEXTPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glDrawArraysEXT" )));
    ldynamic::glDebugMessageCallback  = reinterpret_cast<PFNGLDEBUGMESSAGECALLBACKPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glDebugMessageCallback" )));

    // During init, enable debug output
    glEnable( GL_DEBUG_OUTPUT );

    // -- Initialize OpenGL --
    display_context_create();
    fid glx_initial = context_create();
    vx_window_id = window_create();
    vx_window = vx_window_list[ vx_window_id ];

    std::cout << "Starting renderer\n" ;

    // Set primary thread local context
    vglx_context = vglx_context_list[ vglx_context_id ];
    context_set_current( glx_initial );

    vglx_extensions_string = glXQueryExtensionsString( rx_display, DefaultScreen(rx_display) );

    // Setup shaders
    shader_fragment_test = shader_create( "shader_fragment_test", shader_type::fragment );
    shader_vertex_test = shader_create( "shader_vertex_test", shader_type::vertex );

    shader_compile( shader_fragment_test, shader_fragment_source );
    shader_compile( shader_vertex_test, shader_vertex_source );

    shader_program_test = shader_program_create( "shader_primitive_test" );
    shader_program_attach( shader_program_test, shader_vertex_test );
    shader_program_attach( shader_program_test, shader_fragment_test );
    shader_program_compile( shader_program_test );

    // Initialize vertex array
    ldynamic::glGenVertexArrays(10, vao);

    // Bind the vertex array
    ldynamic::glBindVertexArray(vao[5]);

    // Copy vertecies into vbo
    ldynamic::glGenBuffers( 10, vbo_actives);
    vbo = vbo_actives[5];
    GLint test_color = vbo_actives[6];

    // Set vertex attributes
    // Setup vertex coordiantes
    ldynamic::glBindBuffer(GL_ARRAY_BUFFER, vbo);
    ldynamic::glBufferData(GL_ARRAY_BUFFER, sizeof(mtest_triangle),
                           &mtest_triangle, GL_STATIC_DRAW);


    ldynamic::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                                    3 * sizeof(float), reinterpret_cast<void*>(0));

    // Setup vertex colors
    ldynamic::glBindBuffer( GL_ARRAY_BUFFER, test_color );
    ldynamic::glBufferData( GL_ARRAY_BUFFER, sizeof(mtest_triangle_colors),
                           &mtest_triangle_colors, GL_STATIC_DRAW );
    ldynamic::glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE,
                                    4 * sizeof(float), reinterpret_cast<void*>(0) );

    ldynamic::glEnableVertexAttribArray(0);
    ldynamic::glEnableVertexAttribArray(1);

    // Disabled VSync for performance
    enum
    {
        vsync_adaptive        = -1,
        vsync_no              = 0,
        vsync_double_buffered = 1,
        vsync_triple_buffered = 2
    };
    // GL_EXT_swap_control
    ldynamic::glXSwapIntervalEXT(rx_display, vx_window, vsync_adaptive);

    return true;
}

fhowdit
FUNCTION def::deinitialize()
{
    for (Window x_xwindow : vx_window_list)
    {
        if (x_xwindow != 0)
        {
            XUnmapWindow( rx_display, x_xwindow );
            XDestroyWindow( rx_display, x_xwindow );
        }
    }
    for (GLXContext x_glx_context : vglx_context_list)
    {
        if (x_glx_context != nullptr)
        {
            glXDestroyContext( rx_display, x_glx_context );
        }
    }
    return true;

}

fid
FUNCTION def::display_context_create()
{
    // Setup X11 Window and OpenGL Context
    rx_display = XOpenDisplay(nullptr);
    vx_connection_number = XConnectionNumber(rx_display);
    vx_connection_string[0] = ':';
    vx_connection_string[1] = '0' + static_cast<char>( vx_connection_number );

    if (rx_display == nullptr)
    {
        std::cout << "Could not open X display" << std::endl;
        return false;
    }
    return true;
}

fid
FUNCTION def::window_create()
{
    fid window_id = 0;

    vx_window_attributes.colormap = XCreateColormap( rx_display,
                                                     RootWindow(rx_display,
                                                                vx_buffer_config->screen),
                                                     vx_buffer_config->visual, AllocNone );
    vx_window_attributes.background_pixmap = None ;
    vx_window_attributes.border_pixel      = 0;
    vx_window_attributes.event_mask        = StructureNotifyMask;
    vx_window_attributes.override_redirect = true;

    Window x_window_tmp = XCreateWindow( rx_display,
                               RootWindow( rx_display, vx_buffer_config->screen ),
                               500, 500, 500, 500, 0,
                               vx_buffer_config->depth,
                               InputOutput,
                               vx_buffer_config->visual,
                               CWBorderPixel|CWColormap|CWEventMask,
                               &vx_window_attributes );

    vx_default_screen = DefaultScreen( rx_display );

    // Fullscreen the window if allowed
    wm_state      = XInternAtom( rx_display, "_NET_WM_STATE", true );
    wm_state_fullscreen = XInternAtom( rx_display, "_NET_WM_STATE_FULLSCREEN", true );
    wm_state_maximized_horz = XInternAtom( rx_display, "_NET_WM_STATE_MAXIMIZED_HORZ", true );
    wm_state_maximized_vert = XInternAtom( rx_display, "_NET_WM_STATE_MAXIMIZED_VERT", true );
    wm_state_above = XInternAtom( rx_display, "_NET_WM_STATE_ABOVE", true );
    wm_allowed_actions = XInternAtom( rx_display, "_NET_WM_ALLOWED_ACTIONS", true );

    Atom wm_state_new[3] =
    {
        wm_state_fullscreen,
        wm_state_maximized_horz,
        wm_state_maximized_vert,
    };
    if (wm_state != 0 && wm_state_fullscreen != 0 && wm_state_maximized_horz!= 0 &&
        wm_state_maximized_vert != 0 && wm_state_above != 0)
    {
        XChangeProperty(rx_display, x_window_tmp,
                        wm_state, XA_ATOM, 32,
                        PropModeReplace,
                        reinterpret_cast<unsigned char*>( &wm_state_new ),
                        3);
    }

    // Set the window name
    XStoreName( rx_display, x_window_tmp, "cpp triangle test" );
    XMapWindow( rx_display, x_window_tmp);

    // Start Handling X11 events
    XSelectInput( rx_display, x_window_tmp, ClientMessage );
    // Instruct window manager to permit self-cleanup
    Atom test_atom = 0;
    test_atom = XInternAtom( rx_display, "WM_DELETE_WINDOW", true );
    if (test_atom != 0)
    {
        vx_wm_delete_window = test_atom;
        vx_window_protocols.push_back(test_atom);
        std::cout << "WM_DELETE_WINDOW protocol loaded \n";
        XSetWMProtocols( rx_display, x_window_tmp, vx_window_protocols.data(), fuint32(vx_window_protocols.size()) );
    }

    vx_window_list.push_back( x_window_tmp );
    window_id = vx_window_list.size() - 1;
    return window_id;
}

fhowdit def::window_destroy( fid target )
{
    Window target_window = vx_window_list[ target ];
    XUnmapWindow( rx_display, target_window );
    XDestroyWindow( rx_display, target_window );
    return true;
}

fhowdit
FUNCTION def::display_context_destroy( fid target )
{
    XUnmapWindow( rx_display, vx_window );
    XDestroyWindow( rx_display, vx_window );
    XFree( vx_buffer_config );
    XCloseDisplay( rx_display );
    return true;
}

fid
FUNCTION def::context_create()
{
    using namespace ldynamic;
    GLXContext context_tmp = nullptr;
    fint32 context_id = 0;

    // Load GL core profile
    int vglx_context_attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            None
        };

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


    vglx_fbconfigurations = ldynamic::
    glXChooseFBConfig( rx_display, DefaultScreen(rx_display),
                      vglx_visual_attributes, &vglx_fb_count );
    vglx_fbselection = vglx_fbconfigurations[0];
    vx_buffer_config = glXGetVisualFromFBConfig(rx_display, vglx_fbselection);

    context_tmp =  glXCreateContextAttribsARB(rx_display, vglx_fbselection, nullptr, true, vglx_context_attribs);
    vglx_context_list.push_back(context_tmp);

    context_id = fint32( vglx_context_list.size() - 1 );
    return context_id;
}

fhowdit
FUNCTION def::context_destroy(fid target)
{
    GLXContext target_context = vglx_context_list[ target ];
    glXDestroyContext( rx_display, target_context );
    return true;
}

fhowdit
FUNCTION def::context_set_current(fid target)
{
    GLXContext target_context = vglx_context_list[ target ];
    glXMakeCurrent( rx_display, vx_window, target_context );
    return true;
}

fid
FUNCTION def::shader_create( fstring_view name, shader_type request_type )
{
    using namespace ldynamic;
    fid out_id = -1;
    GLint shader_target = 0;
    char shader_log[512] = {};
    switch (request_type)
    {
        case shader_type::vertex :
            shader_target = glCreateShader( GL_VERTEX_SHADER ); break;
        case shader_type::fragment :
            shader_target = glCreateShader( GL_FRAGMENT_SHADER ); break;
        case shader_type::geometry :
            shader_target = glCreateShader( GL_VERTEX_SHADER ); break;
        default:
            std::cout << "Shader type not implimented \n";
            return -1;
            break;
    }

    out_id = shader_count;
    shader_list[ shader_count ] = shader_target;
    ++shader_count;
    name.copy( static_cast<char*>( shader_names[ out_id ] ),
               name.length() );

    return out_id;
}

fhowdit
FUNCTION def::shader_load( fid target, fpath shader_file, bool binary )
{
    std::cout << "Unimplimnted \n";
    return false;
}

fhowdit
FUNCTION def::shader_compile( fid target, fstring code )
{
    using namespace ldynamic;
    GLuint compile_target = shader_list[ target ];
    GLint compile_success = false;
    char compile_log[512] = {};
    const char*  compile_source_pointer =  code.c_str();
    GLint compile_source_length = static_cast<GLint>( code.length() );

    glShaderSource( compile_target, 1, &compile_source_pointer, &compile_source_length );
    ldynamic::glCompileShader( compile_target );

    glGetShaderiv( compile_target , GL_COMPILE_STATUS, &compile_success);

    glGetShaderInfoLog( compile_target, 512, nullptr, compile_log );
    if (compile_success == false)
    {
        std::cout << "Shader compilation failed, error message: " << compile_log << "\n";
        return false;
    }
    return true;
}

fid
FUNCTION def::shader_program_create( fstring_view name, std::vector<fid> shaders_attach )
{
    using namespace ldynamic;
    GLuint shader_program = glCreateProgram();
    fid out_id = shader_program_count;
    shader_program_list[ out_id ] = shader_program;
    shader_program_names[ out_id ] = name;
    ++shader_program_count;

    for (fid x_shader : shaders_attach)
    {
        glAttachShader( shader_program, x_shader );
    }
    // Not deleting shaders, OpenGL will ignore it until it's detatch anyway

    return out_id;
}

fhowdit
FUNCTION def::shader_program_compile( fid target )
{
    using namespace ldynamic;
    GLint shader_program_target = shader_program_list[ target ];
    GLint out_link_success = false;
    char shader_info_log[512] = {};
    glLinkProgram( shader_program_target );

    // Check shader setup went okay
    glGetProgramiv( shader_program_target, GL_LINK_STATUS, &out_link_success);
    if (out_link_success == false)
    {
        glGetProgramInfoLog( shader_program_target, 512, nullptr, shader_info_log );
        std::cout << "Shader linkage failed, error message: " << shader_info_log << "\n";
        return false;
    }
    else
    {
        std::cout << "Shader linked to program \n";
    }

    return out_link_success;
}

fhowdit
FUNCTION def::shader_program_attach( fid target, fid shader_attached )
{
    using namespace ldynamic;
    GLint shader_program_target = shader_program_list[ target ];
    GLint shader_attached_target = shader_list[ shader_attached ];

    glAttachShader( shader_program_target, shader_attached_target);

    return true;
}

fhowdit
FUNCTION def::shader_program_detach( fid target, fid shader_detatch )
{
    using namespace ldynamic;
    GLint shader_program_target = shader_program_list[ target ];
    GLint shader_detached_target = shader_list[ shader_detatch ];

    glDetachShader( shader_program_target, shader_detached_target);
    return true;
}

fhowdit
FUNCTION def::vertex_buffer_register( fid target,
                                        unique_ptr<ffloat3> backing_buffer,
                                        fuint32 buffer_size )
{
    using namespace ldynamic;
    GLuint vertex_buffer_target = mvertex_buffer_names[ target ];
    vertex_buffer tmp
    {
        .data = backing_buffer.get(),
        .size = buffer_size,
        .usage_pattern = GL_STATIC_DRAW
    };
    mvertex_buffer_list[ vertex_buffer_target ] = tmp;


    // Bind the default vertex array
    glBindVertexArray( mvertex_array_names[0] );

    glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer_target );
    glBufferData( GL_ARRAY_BUFFER, buffer_size, backing_buffer.get(), GL_STATIC_DRAW );

    // Set vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // Reset buffer to avoid clobbering other buffers

    glBindBuffer( GL_ARRAY_BUFFER, 0);
    return true;
}

GLXContext def::get_gl_context() const
{
    return vglx_context;
}

bool def::draw_test_triangle(vfloat4 color)
{
    using namespace ldynamic;
    glUseProgram( shader_program_list[ shader_program_test ] );
    glBindVertexArray(vao[5]);
    ldynamic::glDrawArrays(GL_TRIANGLES, 0, 3);

    return true;
}

bool def::draw_test_circle(vfloat4 p_color)
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

bool def::draw_test_rectangle(vfloat4 p_color)
{
    GLfloat square_width = 200;
    GLfloat square_height = 200;
// (top-right corner anchored)
    GLfloat square_x = (1920.f / 2.f) - (square_width/2.f);
    GLfloat square_y = (1080.f / 2.f) - (square_height/2.f) + 200.f;

    for (int y = int(square_y); y < square_y + square_height; ++y)
    {
        for (int x = int(square_x); x < square_x+square_width; ++x)
        {
            if ( x + (1920*y) > (1920*1080) ) break;
            mbuffer[x + (1920*y)] = mrectangle_color;
        }
    }

    return true;
}

bool def::draw_test_signfield(vfloat4 p_color)
{
    // Performance optimization, can be disabled when it runs fast enough
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

    (void)(circle_radius);

    GLfloat distance = 0;

    GLfloat color_mult = 1; // magic number

    for (int y = 0; y < 1080; ++y)
    {
        for (int x = 0; x < 1920; ++x)
        {
            distance = std::abs( (0.f +x-circle_x ) * ( 0.f +x-circle_x)) + std::abs( (y - circle_y) * (y - circle_y));

            color_mult = ((1.f / circle_radius) * sqrtf(distance) ) * 0.8f;
            mbuffer[x + (1920*y)] = p_color * color_mult;
        }
    }
    buffer_damage_size = 1920*1080;

    // Interferes with fragment shader
    if (progress_x < 1920) progress_x += 5;
    else progress_x = 1920;
    if (progress_y < 1080) ++progress_y += 5;
    else progress_y = 1080;

    glDrawPixels(1920, progress_y, GL_RGBA, GL_FLOAT, mbuffer.get());

    return true;

}

bool def::refresh()
{
    ZoneScopedN("graphics refresh");

    try
    {
        // XEvent processing
        unsigned int vx_pending_events = 0;
        vx_pending_events = XPending(rx_display);
        for (unsigned int i_events = 0; i_events < vx_pending_events; ++i_events)
        {
            // Get the next event.
            XEvent event;
            XNextEvent(rx_display, &event);
            switch (event.type)
            {
                case ClientMessage:
                    // Window Manager requested application exit
                    // This is an oppurtunity to ignore it and just close the window if needed
                    std::cout << "[XServer] Requested application exit \n";
                    if (static_cast<Atom>( event.xclient.data.l[0] ) == vx_wm_delete_window)
                    {
                        global->kill_program = true;
                        return false;
                    }
                    break;
                case DestroyNotify:
                    // throw(1);
                    break;
            }
        }

        // Rendering
        glClearColor( 1.f, 0.5, 1.f, 1.f );
        glClear( GL_COLOR_BUFFER_BIT );

        // Set the coordinate system for proper clip space conversion
        glViewport(0, 0, 1920, 1080);

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        // render.draw_test_rectangle(render.mrectangle_color);
        // render.draw_test_circle(render.mcircle_color);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        draw_test_triangle(mtriangle_color);
        // draw_test_signfield(msignfield_color);

        glXSwapBuffers ( rx_display, vx_window );

        buffer_damage_size = 0;
    }
    catch (...) { throw(1); }
    return true;
}

DESTRUCTOR def::~renderer_opengl()
{
    // Cleanup
    std::cout << "Renderer Cleanup" << std::endl;
    this->deinitialize();
}
