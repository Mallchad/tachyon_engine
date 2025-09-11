
#include "include_core.h"

x11_context* g_x11;

PROC x11_input_init() -> void
{

}

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
    // x11_input_init();
}

void
FUNCTION x11_destroy()
{
    // XFree( vx_buffer_config ); XFree stuffs?
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

    // Fullscreen the window if allowed
    g_x11->wm_state      = XInternAtom( g_x11->server, "_NET_WM_STATE", true );
    g_x11->wm_state_fullscreen = XInternAtom( g_x11->server, "_NET_WM_STATE_FULLSCREEN", true );
    g_x11->wm_state_maximized_horz = XInternAtom( g_x11->server, "_NET_WM_STATE_MAXIMIZED_HORZ", true );
    g_x11->wm_state_maximized_vert = XInternAtom( g_x11->server, "_NET_WM_STATE_MAXIMIZED_VERT", true );
    g_x11->wm_state_above = XInternAtom( g_x11->server, "_NET_WM_STATE_ABOVE", true );
    g_x11->wm_allowed_actions = XInternAtom( g_x11->server, "_NET_WM_ALLOWED_ACTIONS", true );

    Atom wm_state_new[] =
    {
        g_x11->wm_state_fullscreen
    };
    if (g_x11->wm_state != 0 && g_x11->wm_state_fullscreen != 0 && g_x11->wm_state_maximized_horz!= 0 &&
        g_x11->wm_state_maximized_vert != 0 && g_x11->wm_state_above != 0)
    {
        // fullscreen = true;
        // XChangeProperty( g_x11->server, x_window_tmp,
        //                  g_x11->wm_state, XA_ATOM, 32,
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
        g_x11->wm_delete_window = test_atom;
        g_x11->window_protocols.push_back(test_atom);
        tyon_log( "WM_DELETE_WINDOW protocol loaded" );
        XSetWMProtocols(
            g_x11->server,
            x_window_tmp,
            g_x11->window_protocols.data,
            u32(g_x11->window_protocols.size())
        );
    }

    // Start Handling X11 events
    // Buttons are pointer/mouse numbers
    u32 event_mask = ClientMessage  |
    KeyPressMask | KeyReleaseMask |
    ButtonPressMask | PointerMotionMask | StructureNotifyMask;
    XSelectInput( g_x11->server, g_x11->window, event_mask );

    // Fix auto repeat smashing simeltaneous key input
    XAutoRepeatOn( g_x11->server );

    g_x11->window_list.push_back( x_window_tmp );
    window_new = g_x11->window_list.size() - 1;
    return window_new;
}

PROC x11_window_close( window_id target )
{
    Window target_window = g_x11->window_list[ target.cast() ];
    XUnmapWindow( g_x11->server, target_window );
    XDestroyWindow( g_x11->server, target_window );
    return true;
}

PROC x11_event_process()
{
    // XEvent processing
    i32 pending_events = 0;
    pending_events = XPending( g_x11->server );
    g_x11->diagnostic_recent_event_count += pending_events;

    // Report events periodically
    static time_periodic log_timer( 2s );
    if (log_timer.triggered())
    {
        log_format( "X11", "Processing {} events from the server",
                    g_x11->diagnostic_recent_event_count );
        g_x11->diagnostic_recent_event_count = 0;
    }

    for (fint32 i_events = 0; i_events < pending_events; ++i_events)
    {
        // Get the next event.
        XEvent event;
        XNextEvent( g_x11->server, &event );
        switch (event.type)
        {
            case ClientMessage:
                // Window Manager requested application exit
                // This is an oppurtunity to ignore it and just close the window if needed
                // The user may be provided with a conformation dialogue and choose
                // to not exit the applicaiton, ths should be respected.
                // If we choose not to honour the deletion, we have to restart
                // NET_WM_DELETE protocol
                std::cout << "[XServer] Requested application exit \n";
                if (static_cast<Atom>( event.xclient.data.l[0] ) == g_x11->wm_delete_window)
                {
                    global->kill_program = true;
                    return false;
                }
                break;
            case DestroyNotify:
                // throw(1);
                break;
            case KeyPress:
                if ( XKeysymToKeycode( g_x11->server, XK_Escape ) == event.xkey.keycode )
                {
                    global->kill_program = true;
                    return false;
                }
                if ( XKeysymToKeycode( g_x11->server, XK_R ) == event.xkey.keycode &&
                    global->reload_released == true)
                {
                    global->reload_shaders = true;
                    global->reload_released = false;
                }
                if (XKeysymToKeycode( g_x11->server, XK_W ) == event.xkey.keycode)
                { global->action_forward = true; }
                if (XKeysymToKeycode( g_x11->server, XK_S ) == event.xkey.keycode)
                { global->action_backward = true; }
                if (XKeysymToKeycode( g_x11->server, XK_A ) == event.xkey.keycode)
                { global->action_left = true; }
                if (XKeysymToKeycode( g_x11->server, XK_D ) == event.xkey.keycode)
                { global->action_right = true; }
                if (XKeysymToKeycode( g_x11->server, XK_space ) == event.xkey.keycode)
                { global->action_up = true; }
                if (XKeysymToKeycode( g_x11->server, XK_Control_L ) == event.xkey.keycode)
                { global->action_down = true; }
                if (XKeysymToKeycode( g_x11->server, XK_Up ) == event.xkey.keycode)
                { global->action_turn_up = true; }
                if (XKeysymToKeycode( g_x11->server, XK_Down ) == event.xkey.keycode)
                { global->action_turn_down = true; }
                if (XKeysymToKeycode( g_x11->server, XK_Left ) == event.xkey.keycode)
                { global->action_turn_left = true; }
                if (XKeysymToKeycode( g_x11->server, XK_Right ) == event.xkey.keycode)
                { global->action_turn_right = true; }
                break;
            case ConfigureNotify:
                global->window_requested.width = event.xconfigure.width;
                global->window_requested.height = event.xconfigure.height;
                break;
            case KeyRelease:
                if ( XKeysymToKeycode( g_x11->server, XK_R ) == event.xkey.keycode )
                { global->reload_released = true; }
                if (XKeysymToKeycode( g_x11->server, XK_W ) == event.xkey.keycode)
                { global->action_forward = false; }
                if (XKeysymToKeycode( g_x11->server, XK_S ) == event.xkey.keycode)
                { global->action_backward = false; }
                if (XKeysymToKeycode( g_x11->server, XK_A ) == event.xkey.keycode)
                { global->action_left = false; }
                if (XKeysymToKeycode( g_x11->server, XK_D ) == event.xkey.keycode)
                { global->action_right = false; }
                if (XKeysymToKeycode( g_x11->server, XK_space ) == event.xkey.keycode)
                { global->action_up = false; }
                if (XKeysymToKeycode( g_x11->server, XK_Control_L ) == event.xkey.keycode)
                { global->action_down = false; }
                if (XKeysymToKeycode( g_x11->server, XK_Up ) == event.xkey.keycode)
                { global->action_turn_up = false; }
                if (XKeysymToKeycode( g_x11->server, XK_Down ) == event.xkey.keycode)
                { global->action_turn_down = false; }
                if (XKeysymToKeycode( g_x11->server, XK_Left ) == event.xkey.keycode)
                { global->action_turn_left = false; }
                if (XKeysymToKeycode( g_x11->server, XK_Right ) == event.xkey.keycode)
                { global->action_turn_right  = false; }
                break;
            default: break;
        }
    }

    return true;
}

PROC x11_tick()
{
    x11_event_process();
}
