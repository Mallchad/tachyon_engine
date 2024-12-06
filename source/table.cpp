
#include "table.h"
#include "memory.hpp"

lua_value
FUNCTION table::operator[] ( tyon::string key )
{
    lua_value result;
    raw_zero( &result, 1 );
    // Retreive key
    lua_getglobal( lua, name.data );
    i32 table_index = lua_gettop(lua);
    lua_pushstring( lua, key.data );
    lua_gettable( lua, table_index );

    // Conversion
    result.lfloat = lua_tonumber( lua, lua_gettop(lua) );

    if (lua_isnil( lua, -1 ))
    {
        log( "lua", "indexed a nil value, this is probably not what you wanted." );
        return result;
    }
    return result;
}
