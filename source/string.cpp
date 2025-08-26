
#include "include_core.h"

using namespace tyon;

COPY_CONSTRUCTOR tyon::string::string( const char* target )
{
    null_tainted = true;
    owning = true;
    // Compute size with null terminator
    size = 1+ compile_cstring_size( target );
    // Compute reserve with SIMD padding
    reserve = 4+ size;
    data = allocate<char>( reserve );
    raw_copy( data, target, this->size);
}

u32
FUNCTION tyon::string::compile_cstring_size( cstring target )
{
    u32 result = 0;
    for (; target[ result ] != 0; ++result) {}
    return result;
}
