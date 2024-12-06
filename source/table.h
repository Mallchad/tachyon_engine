
#pragma once

extern "C"
{
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
    #include "lstate.h"
};
#include "string.hpp"

union lua_value
{
    struct GCObject *gc;    /* collectable objects */
    void *p;         /* light userdata */
    lua_CFunction lfunction; /* light C functions */
    LUA_INTEGER lint;   /* integer numbers */
    LUA_NUMBER lfloat;    /* float numbers */
    /* not used, but may avoid warnings for uninitialized value */
    fbyte lbyte;


};

struct table
{
    tyon::string name;
    lua_State* lua;

    lua_value
    FUNCTION operator[] ( tyon::string key );
};
