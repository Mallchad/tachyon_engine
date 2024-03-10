
#pragma once

#include "linux_opengl.h"
#include "math.hpp"
#include "global.h"
using renderer_backend = renderer_opengl;

class renderer final
{
    globals* global;
    mesh_id test_utah_teapot_id = -1;
    fmesh test_utah_teapot;
    fpath utah_teapot_file;
    std::vector<ffloat3> test_utah_teapot_vertecies;
    std::vector<ffloat4> test_utah_teapot_colors;

    shader_program_id test_shader = -1;
    shader_id test_shader_frag = -1;
    shader_id test_shader_vert = -1;
    fpath test_shader_frag_file = "test_utah_teapot.frag";
    fpath test_shader_vert_file = "test_utah_teapot.vert";


    frame_shader_global frame_shader_globals;

    public:
    renderer_backend platform = {};

    CONSTRUCTOR renderer();

    void frame_update(ffloat epoch_elapsed);
};
