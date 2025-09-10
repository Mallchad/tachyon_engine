
#include "include_core.h"
#include <X11/Xlib.h>


struct x11_context
{
    /** An x11 'display' can be thought of as more of a server connection,
    though individual server connections can serve different displays in niche
    setups. */
    Display* server = nullptr;
    i32 server_connection_number = -1;
    i32 screen = -1;
    Window root_window = 0;
    Window window = 0;
    array<Atom> window_protocols;
    array<uid> window_list;
};

x11_context* g_x11;

void
FUNCTION x11_init()
{
    g_x11 = memory_allocate<x11_context>( 1 );
    g_x11->server = XOpenDisplay( nullptr );
    if (g_x11->server == nullptr)
    { tyon_error( "Could not open X11 Server connecton" ); return; }
    g_x11->server_connection_number = XConnectionNumber( g_x11->server );

    // Set custom error handler so we can do more graceful error handling
    // XSetErrorHandler( x11_error_handler );
}

void
FUNCTION x11_destroy()
{
    XCloseDisplay( g_x11->server );
    g_x11->~x11_context();
}

window_id
FUNCTION x11_window_open()
{
    window_id window_new = 0;
    XSetWindowAttributes window_attributes = {};
    // Set screen to whole logical display which spans all physical monitors
    g_x11->screen = XDefaultScreen( g_x11->server );
    g_x11->root_window = XRootWindow( g_x11->server, g_x11->screen );
    // XVisualInfo visual_info = {};
    // window_attributes.colormap = XCreateColormap(
    //     g_x11->server,
    //     g_x11->root_window,
    //     visual_info->visual,
    //     AllocNone
    // );
    window_attributes.background_pixmap = None;
    window_attributes.border_pixel      = 0;
    window_attributes.event_mask        = StructureNotifyMask;
    window_attributes.override_redirect = true;

    Window x_window_tmp = XCreateWindow(
        g_x11->server,
        RootWindow( g_x11->server, g_x11->screen ),
        500, 500, 500, 500, 0,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        CWBorderPixel|CWColormap|CWEventMask,
        &window_attributes
    );

    Atom wm_delete_window = 0;
    Atom wm_state = 0;
    Atom wm_state_fullscreen = 0;
    Atom wm_state_maximized_vert = 0;
    Atom wm_state_maximized_horz = 0;
    Atom wm_state_above = 0;
    Atom wm_allowed_actions = 0;

    // Fullscreen the window if allowed
    wm_state      = XInternAtom( g_x11->server, "_NET_WM_STATE", true );
    wm_state_fullscreen = XInternAtom( g_x11->server, "_NET_WM_STATE_FULLSCREEN", true );
    wm_state_maximized_horz = XInternAtom( g_x11->server, "_NET_WM_STATE_MAXIMIZED_HORZ", true );
    wm_state_maximized_vert = XInternAtom( g_x11->server, "_NET_WM_STATE_MAXIMIZED_VERT", true );
    wm_state_above = XInternAtom( g_x11->server, "_NET_WM_STATE_ABOVE", true );
    wm_allowed_actions = XInternAtom( g_x11->server, "_NET_WM_ALLOWED_ACTIONS", true );

    Atom wm_state_new[] =
    {
        wm_state_fullscreen
    };
    if (wm_state != 0 && wm_state_fullscreen != 0 && wm_state_maximized_horz!= 0 &&
        wm_state_maximized_vert != 0 && wm_state_above != 0)
    {
        // fullscreen = true;
        // XChangeProperty( g_x11->server, x_window_tmp,
        //                  wm_state, XA_ATOM, 32,
        //                  PropModeReplace,
        //                  reinterpret_cast<unsigned char*>( &wm_state_new ),
        //                  sizeof(wm_state_new) );
    }

    // Set the window name
    XStoreName( g_x11->server, x_window_tmp, "Tachyon Engine" );
    XMapWindow( g_x11->server, x_window_tmp );

    // Instruct window manager to permit self-cleanup
    Atom test_atom = 0;
    test_atom = XInternAtom( g_x11->server, "WM_DELETE_WINDOW", true );
    if (test_atom != 0)
    {
        wm_delete_window = test_atom;
        g_x11->window_protocols.push_back(test_atom);
        tyon_log( "WM_DELETE_WINDOW protocol loaded" );
        XSetWMProtocols(
            g_x11->server,
            x_window_tmp,
            g_x11->window_protocols.data,
            u32(g_x11->window_protocols.size())
        );
    }

    g_x11->window_list.push_back( x_window_tmp );
    window_new = g_x11->window_list.size() - 1;
    return window_new;
}
