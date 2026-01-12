
#pragma once

extern "C"
{
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
};

#ifndef TRIANGULATE_PROJECT_ROOT
    static_assert( false, "TRIANGULATE_PROJECT_ROOT not defined" );
#endif

namespace tyon
{
    struct window_properties
    {
        int width;
        int height;
    };
}

enum class e_render_backend : i32
{
    none = 0,
    any = 1,
    opengl = 2,
    vulkan = 3,
};

struct globals
{
    using fpath = std::filesystem::path;
    using fstring = std::string;
public:
    static globals* primary_database;

    std::chrono::time_point<std::chrono::steady_clock> program_epoch;
    lua_State* lua_state;

    /// Search paths which serve as a root for a file reference
    std::vector<fstring> search_paths;
    fpath project_root = TRIANGULATE_PROJECT_ROOT;

    // Backend service to use for fulfilling rendering duties
    e_render_backend render_backend = e_render_backend::vulkan;
    // prefer llvmpipe software renderer, either OpenGL or Vulkan
    bool graphics_llvmpipe = false;
    // Insert breakpoint on errors
    bool debugger_mode = false;

    /// Try to gracefully quit the program
    bool kill_program = false;

    /// Try to reload all active shaders
    bool reload_shaders = false;
    bool reload_released = true;

    tyon::window_properties window;
    tyon::window_properties window_requested;

    std::vector<fpath> asset_search_paths = { project_root / "source/shaders" };

    // Movement States
    bool action_forward = false;
    bool action_left = false;
    bool action_right = false;
    bool action_backward = false;
    bool action_up = false;
    bool action_down = false;
    bool action_turn_up = false;
    bool action_turn_down = false;
    bool action_turn_left = false;
    bool action_turn_right = false;

    static globals* get_primary();
};

extern globals* global;
