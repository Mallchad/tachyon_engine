
#pragma once

#include "linux_opengl.h"
#include "math.hpp"
#include "global.h"
typedef renderer_opengl renderer_backend;

class renderer final
{
    global_database* global;
    mesh_id test_utah_teapot_id = -1;
    fmesh test_utah_teapot;
    std::vector<ffloat3> test_utah_teapot_vertecies;
    std::vector<ffloat4> test_utah_teapot_colors;

    shader_program_id test_shader = -1;
    shader_id test_shader_frag = -1;
    shader_id test_shader_vert = -1;

    public:
    renderer_backend platform = {};

    CONSTRUCTOR renderer();

    void frame_update(ffloat epoch_elapsed);
};
