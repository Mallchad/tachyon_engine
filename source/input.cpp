
#include "input.h"

using def = input;

CONSTRUCTOR def::input( renderer_backend& render_handle ) :
    platform( render_handle )
{
}

freport
FUNCTION def::frame_update( ffloat epoch_elapsed )
{
    platform.frame_update( epoch_elapsed );

    return true;
}
