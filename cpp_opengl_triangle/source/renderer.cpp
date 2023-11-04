
#include "renderer.h"

#include "code_helpers.h"
#include "math.hpp"

CONSTRUCTOR renderer::renderer()
{
    platform.create_context();
}

void
FUNCTION renderer::FrameUpdate(ffloat epoch_elapsed)
{
    platform.refresh();
}
