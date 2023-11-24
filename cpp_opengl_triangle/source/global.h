
#pragma once


#include <GL/glx.h>

class global_database
{
public:
    static global_database* primary_database;

    static global_database* get_primary();
    /// Try to gracefully quit the program
    bool kill_program = false;

    /// Try to reload all active shaders
    bool reload_shaders = false;
    bool reload_released = true;
};
