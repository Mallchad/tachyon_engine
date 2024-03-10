
#include "linux_input_xlib.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <iostream>

using impl = input_xlib;

CONSTRUCTOR impl::input_xlib( renderer_opengl& render_handle )
{
    this->initialize( render_handle );
}
freport
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
    ButtonPressMask | PointerMotionMask;
    XSelectInput( m_display, m_window, event_mask );
    return true;
}

freport
FUNCTION impl::frame_update( ffloat epoch_elapsed )
{
    ++m_tmp_frame_count;
    // XEvent processing
    unsigned int pending_events = 0;
    pending_events = XPending( m_display );
    m_events_received += pending_events;

    // Report events every 5 seconds
    if (pending_events > 0 && m_tmp_frame_count > 144 * 5)
    {
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
                // The user may be provided with a conformation disalogue and choose
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

                break;
            case KeyRelease:
                if ( XKeysymToKeycode( m_display, XK_R ) == event.xkey.keycode )
                {
                    global->reload_released = true;
                }
                break;
            default: break;
        }
    }

    return true;
}
