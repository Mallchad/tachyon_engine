
#include "linux_opengl.h"
#include "math.hpp"

typedef renderer_opengl renderer_backend;

class renderer final
{
    renderer_backend platform = {};
    public:
    CONSTRUCTOR renderer();

    void frame_update(ffloat epoch_elapsed);
};
