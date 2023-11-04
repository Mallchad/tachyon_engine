
#include "renderer.h"

#include "code_helpers.h"
#include "math.hpp"

CONSTRUCTOR renderer::renderer()
{
    platform;
}

void
FUNCTION renderer::frame_update(ffloat epoch_elapsed)
{
    (void)(epoch_elapsed);
    platform.refresh();
}
