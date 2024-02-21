#pragma once

#include "linux_input_xlib.h"
#include "renderer.h"
#include "error.hpp"

using input_backend = input_xlib;

class input final
{
    input_backend platform;
public:
    CONSTRUCTOR input( renderer_backend& render_handle );

    freport
    FUNCTION frame_update( ffloat epoch_elapsed );
};
