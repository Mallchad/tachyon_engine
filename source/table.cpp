
#include "include_core.h"

void
COPY_ASSIGNMENT lua_value_proxy::operator= (lua_Number rhs)
{
    lfloat = rhs;
    lua_getglobal( origin->lua, origin->name.c_str() );
    i32 table_index = lua_gettop( origin->lua );
    lua_pushstring( origin->lua, key.c_str() );
    lua_pushnumber( origin->lua, rhs );
    lua_settable( origin->lua, table_index );
    lua_pop( origin->lua, 1 );
}

lua_value_proxy
FUNCTION lua_table::operator[] ( fstring key )
{
    lua_value_proxy result;
    raw_zero( &result, sizeof(result) );
    result.origin = (this);
    result.key = key;
    // Retreive key
    lua_getglobal( lua, name.c_str() );
    i32 table_index = lua_gettop(lua);
    lua_pushstring( lua, key.c_str() );
    lua_gettable( lua, table_index );
    lua_pop( lua, 2 );

    // Conversion
    result.lfloat = lua_tonumber( lua, lua_gettop(lua) );

    if (lua_isnil( lua, -1 ))
    {
        log( "lua", "indexed a nil value, this is probably not what you wanted." );
        return result;
    }
    return result;
}
