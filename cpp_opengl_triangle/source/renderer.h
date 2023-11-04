
#include "linux_opengl.h"
#include "math.hpp"

class renderer final
{
    renderer_backend platform = {};
    public:
    CONSTRUCTOR renderer();

    void FrameUpdate(ffloat epoch_elapsed);
};
