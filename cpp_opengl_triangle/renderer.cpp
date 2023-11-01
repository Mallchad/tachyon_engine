
#include "renderer.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <GL/gl.h>
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
renderer::renderer()
{
    signal(SIGINT, sigterm_handler);
    global = global_database::get_primary();
    if (global == nullptr)
    {
        std::cout << "Could not aquire global database, cannot manage program safely \n";
        throw(1);

    }

    // -- GLX and X11 Initialization --

    glXCreateContextAttribsARB = reinterpret_cast<PFNGLXCREATECONTEXTATTRIBSARBPROC>(
       glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glXCreateContextAttribsARB" )));
    _glXChooseFBConfig= reinterpret_cast<PFNGLXCHOOSEFBCONFIGPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glXChooseFBConfig" )));
    glXSwapIntervalMESA = reinterpret_cast<PFNGLXSWAPINTERVALMESAPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glXSwapIntervalMESA" )));
    glXSwapIntervalEXT = reinterpret_cast<PFNGLXSWAPINTERVALEXTPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glXSwapIntervalEXT" )));

    glBindBuffer        = reinterpret_cast<PFNGLBINDBUFFERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glBindBuffer" )));
    glGenBuffers        = reinterpret_cast<PFNGLGENBUFFERSPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGenBuffers" )));
    glBufferData        = reinterpret_cast<PFNGLBUFFERDATAPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glBufferData" )));
    glCreateShader      = reinterpret_cast<PFNGLCREATESHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glCreateShader" )));
    glShaderSource      = reinterpret_cast<PFNGLSHADERSOURCEPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glShaderSource" )));
    glCompileShader     = reinterpret_cast<PFNGLCOMPILESHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glCompileShader" )));

    glGetShaderiv       = reinterpret_cast<PFNGLGETSHADERIVPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGetShaderiv" )));
    glGetShaderInfoLog  = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGetShaderInfoLog" )));
    glCreateProgram     = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glCreateProgram" )));
    glAttachShader      = reinterpret_cast<PFNGLATTACHSHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glAttachShader" )));
    glDeleteShader      = reinterpret_cast<PFNGLDELETESHADERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glDeleteShader" )));

    glLinkProgram       = reinterpret_cast<PFNGLLINKPROGRAMPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glLinkProgram" )));
    glGetProgramiv      = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGetProgramiv" )));
    glUseProgram = reinterpret_cast<PFNGLUSEPROGRAMPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glUseProgram" )));
    glGetProgramInfoLog = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGetProgramInfoLog" )));

    glVertexAttribPointer=reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glVertexAttribPointer" )));
    glGenVertexArrays = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glGenVertexArrays" ) ));
    glEnableVertexAttribArray = reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glEnableVertexAttribArray" )));
    glEnableVertexArrayAttrib = reinterpret_cast<PFNGLENABLEVERTEXARRAYATTRIBPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glEnableVertexArrayAttrib" )));
    glBindVertexArray = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glBindVertexArray" )));
    glMapBuffer  = reinterpret_cast<PFNGLMAPBUFFERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glMapBuffer" )));
    glUnmapBuffer  = reinterpret_cast<PFNGLUNMAPBUFFERPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glUnmapBuffer" )));

    intern_glDrawArrays = reinterpret_cast<PFNGLDRAWARRAYSPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glDrawArrays" )));
    glDrawArraysEXT = reinterpret_cast<PFNGLDRAWARRAYSEXTPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glDrawArraysEXT" )));
    glDebugMessageCallback  = reinterpret_cast<PFNGLDEBUGMESSAGECALLBACKPROC>(
        glXGetProcAddress( reinterpret_cast<const GLubyte*>( "glDebugMessageCallback" )));


    // During init, enable debug output
    glEnable( GL_DEBUG_OUTPUT );

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
    vx_connection_number = XConnectionNumber(rx_display);
    vx_connection_string[0] = ':';
    vx_connection_string[1] = '0' + static_cast<char>( vx_connection_number );

    if (rx_display == nullptr)
    {
        std::cout << "Could not open X display" << std::endl;
        throw(1);
    }
    vglx_fbconfigurations = _glXChooseFBConfig(rx_display, DefaultScreen(rx_display), vglx_visual_attributes, &vglx_fb_count);
    vglx_fbselection = vglx_fbconfigurations[0];
    vx_buffer_config = glXGetVisualFromFBConfig(rx_display, vglx_fbselection);

    // Load GL core profile
    int vglx_context_attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            None
        };
    vglx_context =  glXCreateContextAttribsARB(rx_display, vglx_fbselection, nullptr, true, vglx_context_attribs);

    vx_window_attributes.colormap = XCreateColormap( rx_display,
                                                     RootWindow(rx_display,
                                                                vx_buffer_config->screen),
                                                     vx_buffer_config->visual, AllocNone );
    vx_window_attributes.background_pixmap = None ;
    vx_window_attributes.border_pixel      = 0;
    vx_window_attributes.event_mask        = StructureNotifyMask;
    vx_window_attributes.override_redirect = true;

    vx_window = XCreateWindow( rx_display,
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

    Atom wm_state_new[4] =
    {
        wm_state_fullscreen,
        wm_state_maximized_horz,
        wm_state_maximized_vert,
        wm_state_above
    };
    if (wm_state != 0 && wm_state_fullscreen != 0 && wm_state_maximized_horz!= 0 &&
        wm_state_maximized_vert != 0 && wm_state_above != 0)
    {
        XChangeProperty(rx_display, vx_window,
                        wm_state, XA_ATOM, 32,
                        PropModeReplace,
                        reinterpret_cast<unsigned char*>( &wm_state_new ),
                        4);
    }

    // Set the window name
    XStoreName( rx_display, vx_window, "cpp triangle test" );
    XMapWindow( rx_display, vx_window );

    glXMakeCurrent( rx_display, vx_window, vglx_context );

    // Start Handling X11 events
    XSelectInput( rx_display, vx_window, ClientMessage );
    // Instruct window manager to permit self-cleanup
    Atom test_atom = 0;
    test_atom = XInternAtom( rx_display, "WM_DELETE_WINDOW", true );
    if (test_atom != 0)
    {
        vx_wm_delete_window = test_atom;
        vx_window_protocols.push_back(test_atom);
        std::cout << "WM_DELETE_WINDOW protocol loaded \n";
        XSetWMProtocols( rx_display, vx_window, vx_window_protocols.data(), vx_window_protocols.size());
    }

    // -- Initialize OpenGL --
    vglx_extensions_string = glXQueryExtensionsString( rx_display, DefaultScreen(rx_display) );
    // Setup shaders
    shader_vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader_vertex, 1, &shader_vertex_source, nullptr);
    glCompileShader(shader_vertex);

    glGetShaderiv(shader_vertex, GL_COMPILE_STATUS, &shader_compiled);
    glGetShaderInfoLog(shader_vertex, 512, nullptr, shader_info_log);
    if (shader_compiled == false)
    {
        std::cout << "Shader compilation failed, error message: " << shader_info_log << "\n";
        throw(1);
    }

    shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader_fragment, 1, &shader_fragment_source, nullptr);
    glCompileShader(shader_fragment);

    glGetShaderiv(shader_fragment, GL_COMPILE_STATUS, &shader_compiled);
    glGetShaderInfoLog(shader_fragment, 512, nullptr, shader_fragment_log);
    if (shader_compiled == false)
    {
        std::cout << "Shader compilation failed, error message: " << shader_info_log << "\n"
                  << shader_fragment_log << "\n";
        throw(1);
    }

    shader_program = glCreateProgram();
    glAttachShader( shader_program, shader_vertex);
    glAttachShader( shader_program, shader_fragment);
    glLinkProgram( shader_program);

    // Done setting up shaders, cleanup
    glDeleteShader( shader_vertex);
    glDeleteShader( shader_fragment);

    // Check shader setup went okay
    glGetProgramiv(shader_program, GL_LINK_STATUS, &shader_link_sucess);
    if (shader_link_sucess == false)
    {
        glGetProgramInfoLog(shader_program, 512, nullptr, shader_info_log);
        std::cout << "Shader linkage failed, error message: " << shader_info_log << "\n";
        throw(1);
    }
    else
    {
        std::cout << "Shader linked to program \n";
    }

    // Initialize vertex array
    glGenVertexArrays(10, vao);

    // Bind the vertex array
    glBindVertexArray(vao[5]);

    // Copy vertecies into vbo
    glGenBuffers( 10, vbo_actives);
    vbo = vbo_actives[5];
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mtest_triangle), &mtest_triangle, GL_STATIC_DRAW);

    // Set vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));

    // glUnmapBuffer(GL_ARRAY_BUFFER);
    glEnableVertexAttribArray(0);

    // Disabled VSync for performance
    enum
    {
        vsync_adaptive        = -1,
        vsync_no              = 0,
        vsync_double_buffered = 1,
        vsync_triple_buffered = 2
    };
    // GL_EXT_swap_control
    glXSwapIntervalEXT(rx_display, vx_window, vsync_adaptive);

}

GLXContext renderer::get_gl_context() const
{
    return vglx_context;
}

bool renderer::draw_test_triangle(vfloat4 color)
{
    glUseProgram(shader_program);
    glBindVertexArray(vao[5]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    intern_glDrawArrays(GL_TRIANGLES, 0, 3);

    return true;
}

bool renderer::draw_test_circle(vfloat4 p_color)
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

bool renderer::draw_test_rectangle(vfloat4 p_color)
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

bool renderer::draw_test_signfield(vfloat4 p_color)
{
    // Performance optimization, can be disabled when it runs fast enough
    if (buffer_damage_size <= 0)
    {
        // return false;
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

            color_mult = ((1.f / circle_radius) * sqrtf(distance) ) * 0.8;
            mbuffer[x + (1920*y)] = p_color * color_mult;
        }
    }
    buffer_damage_size = 1920*1080;

    return true;

}

bool renderer::refresh()
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

        // render.draw_test_rectangle(render.mrectangle_color);
        // render.draw_test_circle(render.mcircle_color);
        draw_test_signfield(msignfield_color);
        // draw_test_triangle(mtriangle_color);

        // Interferes with fragment shader
        if (progress_x < 1920) progress_x += 5;
        else progress_x = 1920;
        if (progress_y < 1080) ++progress_y += 5;
        else progress_y = 1080;

        glDrawPixels(1920, progress_y, GL_RGBA, GL_FLOAT, mbuffer.get());
        glXSwapBuffers ( rx_display, vx_window );

        buffer_damage_size = 0;
    }
    catch (...) { throw(1); }
    return true;
}

DESTRUCTOR renderer::~renderer()
{
// Cleanup
    std::cout << "Renderer Cleanup" << std::endl;

    XUnmapWindow( rx_display, vx_window );
    XDestroyWindow( rx_display, vx_window );
    XFree( vx_buffer_config );
    glXDestroyContext( rx_display, vglx_context );
    XCloseDisplay( rx_display );
}
