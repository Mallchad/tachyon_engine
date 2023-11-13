
#pragma once


#include <GL/glx.h>

class global_database
{
public:
    static global_database* primary_database;

    static global_database* get_primary();
    bool kill_program = false;
};
