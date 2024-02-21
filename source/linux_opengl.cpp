
#include "renderer_interface.hpp"
#include "include_core.h"

#include "linux_opengl.h"

#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

// GL Function Prototypes
#include <GL/glcorearb.h>
#include <GL/glext.h>
#include <GL/glxext.h>

#include <bits/this_thread_sleep.h>
#include <chrono>
#include <cmath>
#include <compare>
#include <iostream>
#include <csignal>
#include <initializer_list>

using namespace std::chrono_literals;
using std::initializer_list;

using def = renderer_opengl;

/// Linkage Dynamic functions with less compatability than simple functions
namespace ldynamic
{
    INTERNAL PFNGLXCHOOSEFBCONFIGPROC glXChooseFBConfig;
    INTERNAL PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;
    INTERNAL PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
    INTERNAL PFNGLXSWAPINTERVALMESAPROC glXSwapIntervalMESA;

    INTERNAL PFNGLBINDBUFFERPROC glBindBuffer;
    INTERNAL PFNGLGENBUFFERSPROC glGenBuffers;
    INTERNAL PFNGLBUFFERDATAPROC glBufferData;

    INTERNAL PFNGLCREATESHADERPROC glCreateShader;
/// void glShaderSource( GLuint shader, GLsizei count, const GLchar **string, const GLint *length )
    INTERNAL PFNGLSHADERSOURCEPROC glShaderSource;
    INTERNAL PFNGLCOMPILESHADERPROC glCompileShader;
    INTERNAL PFNGLGETSHADERIVPROC glGetShaderiv;
    INTERNAL PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
    INTERNAL PFNGLCREATEPROGRAMPROC glCreateProgram;
    INTERNAL PFNGLDELETEPROGRAMPROC glDeleteProgram;
    INTERNAL PFNGLATTACHSHADERPROC glAttachShader;
    INTERNAL PFNGLATTACHSHADERPROC glDetachShader;
    INTERNAL PFNGLLINKPROGRAMPROC glLinkProgram;
    INTERNAL PFNGLGETPROGRAMIVPROC glGetProgramiv;
    INTERNAL PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
    INTERNAL PFNGLDELETESHADERPROC glDeleteShader;
    INTERNAL PFNGLISSHADERPROC glIsShader;
    INTERNAL PFNGLISPROGRAMPROC glIsProgram;

    INTERNAL PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
    INTERNAL PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    INTERNAL PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
    INTERNAL PFNGLUSEPROGRAMPROC glUseProgram;
    INTERNAL PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
    INTERNAL PFNGLDRAWARRAYSEXTPROC glDrawArraysEXT;
    INTERNAL PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
    INTERNAL PFNGLMAPBUFFERPROC glMapBuffer;
    INTERNAL PFNGLUNMAPBUFFERPROC glUnmapBuffer;
    INTERNAL PFNGLDRAWARRAYSPROC glDrawArrays;

    INTERNAL PFNGLOBJECTLABELPROC glObjectLabel;
    INTERNAL PFNGLGETSTRINGPROC glGetString;                                // GL 3.0
    INTERNAL PFNGLGETSTRINGIPROC glGetStringi;                              // GL 3.0
    INTERNAL PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
}

// Try to temrinate gracefully after attempt to kill X window
static void
FUNCTION sigterm_handler(int sig)
{
    (void)(sig);
    global_database* tmp = {};
    tmp = global_database::get_primary();
    tmp->kill_program = true;
    std::cout << "caught a signal\n" << std::flush;
    std::signal(SIGINT, sigterm_handler);
}

void
FUNCTION gl_debug_callback( GLenum source,
                            GLenum type,
                            GLuint id,
                            GLenum severity,
                            GLsizei length,
                            const GLchar* message,
                            const void* user_param )
{
    std::cout << "[OpenGL Debug] " << std::left << 
        message << "  Type: " << type << "\n";
}

CONSTRUCTOR def::renderer_opengl()
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
    ldynamic::glDeleteProgram     = reinterpret_cast<PFNGLDELETEPROGRAMPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glDeleteProgram" )));
    ldynamic::glAttachShader      = reinterpret_cast<PFNGLATTACHSHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glAttachShader" )));
    ldynamic::glDetachShader      =  reinterpret_cast<PFNGLATTACHSHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glDetachShader" )));
    ldynamic::glDeleteShader      = reinterpret_cast<PFNGLDELETESHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glDeleteShader" )));
    ldynamic::glIsShader          = reinterpret_cast<PFNGLISSHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glIsShader" )));

    ldynamic::glLinkProgram       = reinterpret_cast<PFNGLLINKPROGRAMPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glLinkProgram" )));
    ldynamic::glGetProgramiv      = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGetProgramiv" )));
    ldynamic::glUseProgram = reinterpret_cast<PFNGLUSEPROGRAMPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glUseProgram" )));
    ldynamic::glGetProgramInfoLog = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGetProgramInfoLog" )));
    ldynamic::glIsProgram          = reinterpret_cast<PFNGLISPROGRAMPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glIsProgram" )));

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

    ldynamic::glObjectLabel  = reinterpret_cast<PFNGLOBJECTLABELPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glObjectLabel" )));
    ldynamic::glGetString  = reinterpret_cast<PFNGLGETSTRINGPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGetString" )));
    ldynamic::glGetStringi  = reinterpret_cast<PFNGLGETSTRINGIPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGetStringi" )));
    ldynamic::glDebugMessageCallback  = reinterpret_cast<PFNGLDEBUGMESSAGECALLBACKPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glDebugMessageCallback" )));

    // -- Initialize OpenGL --
    display_context_create();
    vglx_extensions_string = glXQueryExtensionsString( rx_display, DefaultScreen(rx_display) );
    context_id glx_initial = context_create();
    vx_window_id = window_create();
    vx_window = vx_window_list[ vx_window_id.cast() ];

    std::cout << "Starting renderer\n" ;

    // Set primary thread local context
    vglx_context = vglx_context_list[ vglx_context_id.cast() ];
    context_set_current( glx_initial );

    // Enable debug output
    // glEnable( GL_DEBUG_OUTPUT );
    ldynamic::glDebugMessageCallback( gl_debug_callback, 0 );

    glGetIntegerv( GL_NUM_EXTENSIONS, &m_gl_extension_count );
    std::cout << "OpenGL Implimentation Vendor: " << glGetString( GL_VENDOR ) <<
        "\nOpenGL Renderer String: " << glGetString( GL_RENDERER ) <<
        "\nOpenGL Version: " << glGetString( GL_VERSION ) <<
        "\nOpenGL Shading Language Version: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) <<
        "\nOpenGL Extensions String: ";
    for (fint32 i_extension=0; i_extension<m_gl_extension_count; i_extension += 3)
    {
        std::cout << ldynamic::glGetStringi( GL_EXTENSIONS, 0+ i_extension ) << " ";
        if (i_extension % 3) { std::cout << "\n"; };
    }
    std::cout << "\nGLX Extensions String: " << vglx_extensions_string << "\n";

    // Setup OpenGL Objects
    ldynamic::glGenVertexArrays( mattribute_limit, mattribute_names.data() );
    ldynamic::glGenBuffers( mbuffer_limit, mbuffer_names.data() );

    // Setup test articles
    shader_fragment_test = shader_create( "shader_fragment_test", shader_type::fragment );
    shader_vertex_test = shader_create( "shader_vertex_test", shader_type::vertex );

    shader_compile( shader_fragment_test, shader_fragment_source );
    shader_compile( shader_vertex_test, shader_vertex_source );

    shader_program_test = shader_program_create( "shader_primitive_test" );
    shader_program_attach( shader_program_test, shader_vertex_test );
    shader_program_attach( shader_program_test, shader_fragment_test );
    shader_program_compile( shader_program_test );

    fmesh test_triangle =
    {
        .name = "test_triangle",
        .vertex_buffer = reinterpret_cast<ffloat3*>( mtest_triangle ),
        .vertex_color_buffer = reinterpret_cast<ffloat4*>( mtest_triangle_colors ),
        .vertex_count = 3,
        .color_count = 3,
        .shader_program_id = shader_program_test
    };
    mtest_triangle_mesh = mesh_create( test_triangle );

    // Disabled VSync for performance
    enum
    {
        vsync_adaptive        = -1,
        vsync_no              = 0,
        vsync_double_buffered = 1,
        vsync_triple_buffered = 2
    };
    // GL_EXT_swap_control
    // Can CRASH if extension is not supported
    // ldynamic::glXSwapIntervalEXT( rx_display, vx_window, vsync_adaptive );
    // GLX_MESA_swap_control
    ldynamic::glXSwapIntervalMESA( vsync_adaptive );

    return true;
}

fhowdit
FUNCTION def::deinitialize()
{
    XFree( vx_buffer_config );
    XFree( vglx_fbconfigurations );
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
    XCloseDisplay( rx_display );

    return true;

}

display_id
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

window_id
FUNCTION def::window_create()
{
    window_id window_new = 0;

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
    window_new = vx_window_list.size() - 1;
    return window_new;
}

fhowdit def::window_destroy( window_id target )
{
    Window target_window = vx_window_list[ target.cast() ];
    XUnmapWindow( rx_display, target_window );
    XDestroyWindow( rx_display, target_window );
    return true;
}

fhowdit
FUNCTION def::display_context_destroy( display_id target )
{
    (void)(target);
    XUnmapWindow( rx_display, vx_window );
    XDestroyWindow( rx_display, vx_window );
    XFree( vx_buffer_config );
    XCloseDisplay( rx_display );
    return true;
}

context_id
FUNCTION def::context_create()
{
    using namespace ldynamic;
    GLXContext context_tmp = nullptr;
    fint32 context_id = 0;

    // Load GL core profile
    int vglx_context_attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, vglx_major,
            GLX_CONTEXT_MINOR_VERSION_ARB, vglx_minor,
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


    vglx_fbconfigurations =
        ldynamic::glXChooseFBConfig( rx_display, DefaultScreen(rx_display),
                                     vglx_visual_attributes, &vglx_fb_count );
    vglx_fbselection = vglx_fbconfigurations[0];
    vx_buffer_config = glXGetVisualFromFBConfig(rx_display, vglx_fbselection);

    // context_tmp = glXCreateContextAttribs(rx_display, vglx_fbselection, GLA_RGBA_TYPE, 0, true);
    context_tmp = glXCreateContextAttribsARB(rx_display, vglx_fbselection, nullptr, true, vglx_context_attribs);
    vglx_context_list.push_back(context_tmp);

    context_id = fint32( vglx_context_list.size() - 1 );
    return context_id;
}

fhowdit
FUNCTION def::context_destroy(context_id target)
{
    GLXContext target_context = vglx_context_list[ target.cast() ];
    glXDestroyContext( rx_display, target_context );
    return true;
}

fhowdit
FUNCTION def::context_set_current(context_id target)
{
    GLXContext target_context = vglx_context_list[ target.cast() ];
    glXMakeCurrent( rx_display, vx_window, target_context );
    return true;
}

shader_id
FUNCTION def::shader_create( fstring name, shader_type request_type )
{
    using namespace ldynamic;
    shader_id out_id = -1;
    GLint shader_target = 0;
    char shader_log[512] = {};
    fint32 shader_log_size = 0;
    fstring shader_debug_name;

    // Clear errors
    while (glGetError() != GL_NO_ERROR);

    switch (request_type)
    {
        case shader_type::vertex :
            shader_target = glCreateShader( GL_VERTEX_SHADER );
            shader_debug_name = "vs_"s + name;
            break;
        case shader_type::fragment :
            shader_target = glCreateShader( GL_FRAGMENT_SHADER );
            shader_debug_name = "fs_"s + name;
            break;
        case shader_type::geometry :
            shader_target = glCreateShader( GL_VERTEX_SHADER );
            shader_debug_name = "gs_"s + name;
            break;
        default:
            std::cout << "Shader type not implimented \n";
            return -1;
            break;
    }
    glGetShaderInfoLog( shader_target, 512, &shader_log_size, shader_log );
    if (shader_log_size > 0)
    {
        std::cout << "Shader Log: " << shader_log << "\n";
    }
    
    if ( ldynamic::glIsShader( shader_target ) == false )
    {
        std::cout << "Something went wrong in creating the shader \n";
        return -1;
    }

    glObjectLabel( GL_SHADER, shader_target, shader_debug_name.size(), shader_debug_name.c_str() );
    fstring error_message;
    GLenum error = 0;
    switch (glGetError())
    {
        case GL_NO_ERROR:
            error_message = "GL_NO_ERROR"; break;
        case GL_INVALID_ENUM:
            error_message = "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE:
            error_message = "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:
            error_message = "GL_INVALID_ENUM"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error_message = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
    }
    std::cout << "Shader Creation Error Status: " << error_message << "\n";

    out_id = shader_count;
    shader_list[ shader_count ] = shader_target;
    ++shader_count;

    shader_debug_name.copy( static_cast<char*>( shader_names[ out_id.cast() ] ),
                            shader_debug_name.length() );

    return out_id;
}

fhowdit
FUNCTION def::shader_program_destroy( shader_program_id target )
{
    GLint doomed_program = shader_program_list[ target.cast() ];
    ldynamic::glDeleteProgram( doomed_program );
    return true;
}

fhowdit
FUNCTION def::shader_load( shader_id target, fpath shader_file, bool binary )
{
    std::cout << "Unimplimnted \n";
    return false;
}

fhowdit
FUNCTION def::shader_compile( shader_id target, fstring code )
{
    using namespace ldynamic;
    GLuint compile_target = shader_list[ target.cast() ];
    GLint compile_success = false;
    char compile_log[512] = {};
    const char*  compile_source_pointer =  code.c_str();
    GLint compile_source_length = static_cast<GLint>( code.length() );

    glShaderSource( compile_target, 1, &compile_source_pointer, &compile_source_length );
    ldynamic::glCompileShader( compile_target );

    glGetShaderiv( compile_target , GL_COMPILE_STATUS, &compile_success);

    glGetShaderInfoLog( compile_target, 512, nullptr, compile_log );
    if (compile_success == false || code.length() < 10)
    {
        std::cout << "[Renderer] Shader compilation failed, error message: " << compile_log << "\n";
        return false;
    }
    else
    {
        std::cout << "Compiled Shader[id:" << target.cast() << "]: " <<
            shader_names[ target.cast() ] << "\n";
    }
    return true;
}

shader_program_id
FUNCTION def::shader_program_create( fstring name, initializer_list<shader_id> shaders_attach )
{
    using namespace ldynamic;
    GLuint shader_program = glCreateProgram();
    shader_program_id out_id = shader_program_count;
    shader_program_list[ out_id.cast() ] = shader_program;
    shader_program_names[ out_id.cast() ] = name;
    ++shader_program_count;

    if (ldynamic::glIsProgram( shader_program ) == false)
    {
        std::cout << "[Renderer] shader_program_create | for some reason created "
            "OpenGL shader program is not actually a program \n";
        return -1;
    }

    for (shader_id x_shader : shaders_attach)
    {
        glAttachShader( shader_program, shader_list[ x_shader.cast() ] );
    }
    // Not deleting shaders, OpenGL will ignore it until it's detatch anyway
    name = "program_" + name;
    glObjectLabel( GL_SHADER, shader_program, name.size(), name.c_str() );

    return out_id;
}

fhowdit
FUNCTION def::shader_program_compile( shader_program_id target )
{
    using namespace ldynamic;
    GLint shader_program_target = shader_program_list[ target.cast<fuint32>() ];
    GLint out_link_success = false;
    char shader_info_log[512] = {};
    glLinkProgram( shader_program_target );

    // Check shader setup went okay
    glGetProgramiv( shader_program_target, GL_LINK_STATUS, &out_link_success);
    if (out_link_success == false)
    {
        glGetProgramInfoLog( shader_program_target, 512, nullptr, shader_info_log );
        std::cout << "Shader program linkage failed, error message[id:" <<
                  target.cast() << "] :" << shader_info_log << "\n";
        return false;
    }
    else
    {
        std::cout << "Linked Shader Program[id:" << target.cast() << "|gl:" <<
            shader_program_target << "]: " <<
            shader_program_names[ target.cast() ] << "\n";
    }

    return out_link_success;
}

fhowdit
FUNCTION def::shader_program_attach( shader_program_id target, shader_id shader_attached )
{
    using namespace ldynamic;
    GLint shader_program_target = shader_program_list[ target.cast() ];
    GLint shader_attached_target = shader_list[ shader_attached.cast() ];

    glAttachShader( shader_program_target, shader_attached_target);

    return true;
}

fhowdit
FUNCTION def::shader_program_detach( shader_program_id target, shader_id shader_detatch )
{
    using namespace ldynamic;
    GLint shader_program_target = shader_program_list[ target.cast() ];
    GLint shader_detached_target = shader_list[ shader_detatch.cast() ];

    glDetachShader( shader_program_target, shader_detached_target);
    return true;
}

fhowdit
FUNCTION def::shader_program_run( shader_program_id target )
{
    using namespace ldynamic;
    GLint program_target = shader_program_list[ target.cast() ];
    glUseProgram( program_target );

    return true;
}

mesh_id
FUNCTION def::mesh_create( fmesh target )
{
    using namespace ldynamic;

    fmesh_metadata metadata;
    GLint attributes;
    GLint vertex_buffer;
    GLint index_buffer;
    GLint color_buffer;

    metadata =
        {
            .name                   = target.name,
            .reference_id           = mmesh_count,
            .vertex_attribute_id    = mattribute_count,
            .vertex_buffer_id       = 0+ mbuffer_count,
            .vertex_index_buffer_id = 1+ mbuffer_count,
            .vertex_color_buffer_id = 2+ mbuffer_count,
            .usage_pattern          = GL_STATIC_DRAW
        };
    ++mattribute_count;
    mbuffer_count += 3;
    ++mmesh_count;

    attributes = mattribute_names[ metadata.vertex_attribute_id.cast() ];
    vertex_buffer = mbuffer_names[ metadata.vertex_buffer_id.cast() ];
    index_buffer = mbuffer_names[ metadata.vertex_index_buffer_id.cast() ];
    color_buffer = mbuffer_names[ metadata.vertex_color_buffer_id.cast() ];

    // Set Debug Labels
    fstring attribute_name = "attributes_" + target.name;
    fstring vertex_name = "buffer_vertex_"s + target.name;
    fstring index_name = "buffer_vertex_index_"s + target.name;
    fstring color_name = "buffer_vertex_color_"s + target.name;

    // Register OpenGL buffer and upload the data
    glBindVertexArray( attributes );
    // Vertecies
    glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData( GL_ARRAY_BUFFER, target.vertex_count * sizeof(ffloat3),
                  target.vertex_buffer, GL_STATIC_DRAW );
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(ffloat3), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    glObjectLabel( GL_VERTEX_ARRAY, attributes, attribute_name.size(), attribute_name.c_str() );
    glObjectLabel( GL_BUFFER, vertex_buffer, vertex_name.size(), vertex_name.c_str() );
    if (target.index_count > 0)
    {
        // Vertex indices
        glBindBuffer( GL_ARRAY_BUFFER, index_buffer);
        glBufferData( GL_ARRAY_BUFFER, target.index_count * sizeof(fint32),
                      target.vertex_index_buffer, GL_STATIC_DRAW );
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE,
                              sizeof(fuint32), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);

        glObjectLabel( GL_BUFFER, index_buffer, index_name.size(), index_name.c_str() );
    }
    if (target.color_count > 0)
    {
        // Vertex colors
        glBindBuffer( GL_ARRAY_BUFFER, color_buffer );
        glBufferData( GL_ARRAY_BUFFER, target.color_count * sizeof(rgba),
                      target.vertex_color_buffer, GL_STATIC_DRAW );
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE,
                              sizeof(rgba), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(2);

        glObjectLabel( GL_BUFFER, color_buffer, color_name.size(), color_name.c_str() );
    }

    mmesh_list[ metadata.reference_id.cast() ] = target;
    mmesh_metadata_list[ metadata.reference_id.cast() ] = metadata;
    // Reset to avoid clobbering other buffers
    glBindBuffer( GL_ARRAY_BUFFER, 0);
    glBindVertexArray( 0 );

    return metadata.reference_id;
}

fhowdit
FUNCTION def::draw_mesh( mesh_id target, ftransform target_transform, shader_program_id target_shader )
{
    using namespace ldynamic;
    if (target < 0)
    {
        std::cout << "Something went went, draw_mesh was passed an invalid target \n";
    }
    fmesh target_mesh = mmesh_list [ target.cast() ];
    fmesh_metadata target_meta = mmesh_metadata_list[ target.cast() ];
    shader_program_id shader = target_shader;

    GLint attributes = mattribute_names[ target_meta.vertex_attribute_id.cast() ];

    shader_program_run( shader );
    glBindVertexArray( attributes );
    ldynamic::glDrawArrays( GL_TRIANGLES, 0, target_mesh.vertex_count );

    // Unbind to avoid clobbering other procedures
    glBindVertexArray( 0 );

    return true;
}

GLXContext def::get_gl_context() const
{
    return vglx_context;
}

bool
FUNCTION def::draw_test_triangle(ffloat4 color)
{
    ftransform stub_transform = {};
    draw_mesh( mtest_triangle_mesh, stub_transform, shader_program_test );

    return true;
}

bool
FUNCTION def::draw_test_circle(ffloat4 p_color)
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

bool
FUNCTION def::draw_test_rectangle(ffloat4 p_color)
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

bool
FUNCTION def::draw_test_signfield(ffloat4 p_color)
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

freport
FUNCTION def::frame_start()
{
    // Rendering
    // glClearColor( 1.f, 0.5, 1.f, 1.f );
    glClearColor( 0.f, 0.0f, 0.f, .5f );
    glClear( GL_COLOR_BUFFER_BIT );

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    return true;
}

bool
FUNCTION def::refresh()
{

    // Map the render target to the window width
    glViewport(0, 0, 1920, 1080);

    // render.draw_test_rectangle(render.mrectangle_color);
    // render.draw_test_circle(render.mcircle_color);

    // glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    // glPolygonMode( GL_BACK, GL_LINE ); // Disabled front or back only disabled in profile

    draw_test_triangle(mtriangle_color);
    // draw_test_signfield(msignfield_color);

    glXSwapBuffers ( rx_display, vx_window );

    buffer_damage_size = 0;

    return true;
}

DESTRUCTOR def::~renderer_opengl()
{
    // Cleanup
    std::cout << "Renderer Cleanup" << std::endl;
    this->deinitialize();
}
