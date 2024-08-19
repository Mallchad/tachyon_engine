
#pragma once

#include <vector>
#include <filesystem>
#include <chrono>

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
struct globals
{
    using fpath = std::filesystem::path;
    using fstring = std::string;
public:
    static globals* primary_database;

    std::chrono::time_point<std::chrono::steady_clock> program_epoch;

    /// Search paths which serve as a root for a file reference
    std::vector<fstring> search_paths;
    fpath project_root = TRIANGULATE_PROJECT_ROOT;

    /// Try to gracefully quit the program
    bool kill_program = false;

    /// Try to reload all active shaders
    bool reload_shaders = false;
    bool reload_released = true;

    tyon::window_properties window;
    tyon::window_properties window_requested;

    static globals* get_primary();
};
