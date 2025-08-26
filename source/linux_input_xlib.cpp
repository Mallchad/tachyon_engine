
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <include_core.h>

using impl = input_xlib;
using namespace tyon;

CONSTRUCTOR impl::input_xlib( renderer_opengl& render_handle )
{
    this->initialize( render_handle );
}
fresult
FUNCTION impl::initialize( renderer_opengl& render_handle )
{
    global = globals::get_primary();
    m_renderer = &render_handle;
    m_display = m_renderer->rx_display;
    m_wm_delete_window = m_renderer->vx_wm_delete_window;
    m_window = m_renderer->vx_window;

    // Start Handling X11 events
    // Buttons are pointer/mouse numbers
    fuint32 event_mask = ClientMessage  |
    KeyPressMask | KeyReleaseMask |
    ButtonPressMask | PointerMotionMask | StructureNotifyMask;
    XSelectInput( m_display, m_window, event_mask );

    // Fix auto repeat smashing simeltaneous key input
    XAutoRepeatOn( m_display );

    return true;
}

fresult
FUNCTION impl::frame_update( f32 epoch_elapsed )
{
    ++m_tmp_frame_count;
    // XEvent processing
    fint32 pending_events = 0;
    pending_events = XPending( m_display );
    m_events_received += pending_events;

    // Report events every 5 seconds
    static monotonic_time last_update = get_time();
    auto xserver_update_period = 2s;
    bool log_update_5s = (pending_events > 0) && (get_time() - last_update > xserver_update_period);
    if (log_update_5s)
    {
        last_update = get_time();
        std::cout << "[XServer] Processing " << m_events_received << " events from the server \n";
        m_events_received = 0;
        m_tmp_frame_count = 0;
    }

    for (fint32 i_events = 0; i_events < pending_events; ++i_events)
    {
        // Get the next event.
        XEvent event;
        XNextEvent( m_display, &event );
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
                if (static_cast<Atom>( event.xclient.data.l[0] ) == m_wm_delete_window)
                {
                    global->kill_program = true;
                    return false;
                }
                break;
            case DestroyNotify:
                // throw(1);
                break;
            case KeyPress:
                if ( XKeysymToKeycode( m_display, XK_Escape ) == event.xkey.keycode )
                {
                    global->kill_program = true;
                    return false;
                }
                if ( XKeysymToKeycode( m_display, XK_R ) == event.xkey.keycode &&
                    global->reload_released == true)
                {
                    global->reload_shaders = true;
                    global->reload_released = false;
                }
                if (XKeysymToKeycode( m_display, XK_W ) == event.xkey.keycode)
                { global->action_forward = true; }
                if (XKeysymToKeycode( m_display, XK_S ) == event.xkey.keycode)
                { global->action_backward = true; }
                if (XKeysymToKeycode( m_display, XK_A ) == event.xkey.keycode)
                { global->action_left = true; }
                if (XKeysymToKeycode( m_display, XK_D ) == event.xkey.keycode)
                { global->action_right = true; }
                if (XKeysymToKeycode( m_display, XK_space ) == event.xkey.keycode)
                { global->action_up = true; }
                if (XKeysymToKeycode( m_display, XK_Control_L ) == event.xkey.keycode)
                { global->action_down = true; }
                if (XKeysymToKeycode( m_display, XK_Up ) == event.xkey.keycode)
                { global->action_turn_up = true; }
                if (XKeysymToKeycode( m_display, XK_Down ) == event.xkey.keycode)
                { global->action_turn_down = true; }
                if (XKeysymToKeycode( m_display, XK_Left ) == event.xkey.keycode)
                { global->action_turn_left = true; }
                if (XKeysymToKeycode( m_display, XK_Right ) == event.xkey.keycode)
                { global->action_turn_right = true; }
                break;
            case ConfigureNotify:
                global->window_requested.width = event.xconfigure.width;
                global->window_requested.height = event.xconfigure.height;
                break;
            case KeyRelease:
                if ( XKeysymToKeycode( m_display, XK_R ) == event.xkey.keycode )
                { global->reload_released = true; }
                if (XKeysymToKeycode( m_display, XK_W ) == event.xkey.keycode)
                { global->action_forward = false; }
                if (XKeysymToKeycode( m_display, XK_S ) == event.xkey.keycode)
                { global->action_backward = false; }
                if (XKeysymToKeycode( m_display, XK_A ) == event.xkey.keycode)
                { global->action_left = false; }
                if (XKeysymToKeycode( m_display, XK_D ) == event.xkey.keycode)
                { global->action_right = false; }
                if (XKeysymToKeycode( m_display, XK_space ) == event.xkey.keycode)
                { global->action_up = false; }
                if (XKeysymToKeycode( m_display, XK_Control_L ) == event.xkey.keycode)
                { global->action_down = false; }
                if (XKeysymToKeycode( m_display, XK_Up ) == event.xkey.keycode)
                { global->action_turn_up = false; }
                if (XKeysymToKeycode( m_display, XK_Down ) == event.xkey.keycode)
                { global->action_turn_down = false; }
                if (XKeysymToKeycode( m_display, XK_Left ) == event.xkey.keycode)
                { global->action_turn_left = false; }
                if (XKeysymToKeycode( m_display, XK_Right ) == event.xkey.keycode)
                { global->action_turn_right  = false; }
                break;
            default: break;
        }
    }

    return true;
}
