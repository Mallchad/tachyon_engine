
#include "include_core.h"

using def = input;

CONSTRUCTOR def::input( renderer_backend& render_handle ) :
    platform( render_handle )
{
}

fresult
FUNCTION def::frame_update( f32 epoch_elapsed )
{
    platform.frame_update( epoch_elapsed );

    return true;
}
