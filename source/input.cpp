
#include "include_core.h"

namespace tyon
{

CONSTRUCTOR input::input( renderer_backend& render_handle ) :
    platform( render_handle )
{
}

fresult
FUNCTION input::frame_update( f32 epoch_elapsed )
{
    platform.frame_update( epoch_elapsed );

    return true;
}

}
