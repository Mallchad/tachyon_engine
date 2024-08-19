
#include "error.hpp"

/// Runtime assertion that raises an exception when false
// Aims to improve over the built-in assert by not adhereing to DEBUG/NDEBUG macros
// and being more transparently configurable.
void
FUNCTION assertion( bool expression, cstring message )
{
    print( message );
    std::raise( SIGABRT );
}

void
FUNCTION unimplimented( cstring reason )
{
    print( "This code path is unimplimented, reasoning is as follows " );
    assertion( true, reason );
}
