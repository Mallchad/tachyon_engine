
#include "input.h"

typedef input impl;

CONSTRUCTOR impl::input( renderer_backend& render_handle ) :
    platform( render_handle )
{
}

freport
FUNCTION impl::frame_update( ffloat epoch_elapsed )
{
    platform.frame_update( epoch_elapsed );

    return true;
}
