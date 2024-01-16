
#pragma once
#include "include_core.h"

#ifndef TRIANGULATE_PROJECT_ROOT
    static_assert( false, "TRIANGULATE_PROJECT_ROOT not defined" );
#endif

class global_database
{
public:
    static global_database* primary_database;

    /// Search paths which serve as a root for a file reference
    std::vector<fstring> search_paths;
    IMMEDIATE fstring_view project_root = TRIANGULATE_PROJECT_ROOT;
    IMMEDIATE bool project_root_defined = project_root.size() > 0;

    /// Try to gracefully quit the program
    bool kill_program = false;

    /// Try to reload all active shaders
    bool reload_shaders = false;
    bool reload_released = true;

    static global_database* get_primary();
};
