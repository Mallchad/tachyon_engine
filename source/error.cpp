
#include "include_core.h"

/// Runtime assertion that raises an exception when false
// Aims to improve over the built-in assert by not adhereing to DEBUG/NDEBUG macros
// and being more transparently configurable.
void
FUNCTION assertion( bool expression, cstring message )
{
    tyon_error( message );
    std::raise( SIGABRT );
}

void
FUNCTION unimplimented( cstring reason )
{
    tyon_error( "This code path is unimplimented, reasoning is as follows " );
    assertion( true, reason );
}
