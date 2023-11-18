
#include "linux_opengl.h"
#include "math.hpp"

typedef renderer_opengl renderer_backend;

class renderer final
{
    renderer_backend platform = {};

    fid test_utah_teapot_id = -1;
    fmesh test_utah_teapot;
    std::vector<ffloat3> test_utah_teapot_vertecies;
    std::vector<ffloat4> test_utah_teapot_colors;

    public:
    CONSTRUCTOR renderer();

    void frame_update(ffloat epoch_elapsed);
};
