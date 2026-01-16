#pragma once

namespace tyon
{

using input_backend = input_xlib;

class input final
{
    input_backend platform;
public:
    CONSTRUCTOR input( renderer_backend& render_handle );

    fresult
    FUNCTION frame_update( f32 epoch_elapsed );
};

}
