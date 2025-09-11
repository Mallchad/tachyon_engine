
#pragma once

struct x11_context
{
    /** An x11 'display' can be thought of as more of a server connection,
    though individual server connections can serve different displays in niche
    setups. */
    Display* server = nullptr;
    i32 server_connection_number = -1;
    i32 screen = -1;
    Window root_window = 0;
    // Primary window being used
    Window window = 0;
    array<Atom> window_protocols;
    array<uid> window_list;

    i32 diagnostic_recent_event_count = 0;

    // Property Lookup Cache
    Atom wm_delete_window = 0;
    Atom wm_state = 0;
    Atom wm_state_fullscreen = 0;
    Atom wm_state_maximized_vert = 0;
    Atom wm_state_maximized_horz = 0;
    Atom wm_state_above = 0;
    Atom wm_allowed_actions = 0;
};


void
x11_init();

void
x11_destroy();

window_id
x11_window_open();
