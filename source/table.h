
#pragma once

extern "C"
{
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
    #include "lstate.h"
};
#include "string.hpp"

FORWARD struct lua_value;
FORWARD struct lua_table;

/// Proxy object to allow easily writing to Lua tables
struct lua_value_proxy
{
    lua_table* origin;
    tyon::string key;
    union
    {
        void* p;         /* light userdata */
        lua_CFunction lfunction; /* light C functions */
        lua_Integer lint;   /* integer numbers */
        lua_Number lfloat;    /* float numbers */
        /* not used, but may avoid warnings for uninitialized value */
        fbyte lbyte;


    };
    void
    COPY_ASSIGNMENT operator= (lua_Number rhs);
};

struct lua_table
{
    tyon::string name;
    lua_State* lua;

    lua_value_proxy
    FUNCTION operator[] ( tyon::string key );
};
