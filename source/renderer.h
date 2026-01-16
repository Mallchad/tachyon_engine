
#pragma once

namespace tyon
{

using renderer_backend = renderer_opengl;

class renderer final
{
    globals* global;
    /// The primary camera for the player
    matrix camera;
    // == Test articles ==
    // Triangle Articles
    shader_id shader_fragment_test = -1;
    shader_id shader_vertex_test = -1;
    shader_program_id shader_program_triangle = -1;

    mesh_id mtest_triangle_mesh = -1;
    // 0.433 pre-computed magic number for unit triangle
    // interleaved normal/face
    f32 mtest_triangle[18] =
    {
        0.f, 0.f, -1.f,
        -0.5f, -0.4330127019f, 0.0f,
        0.f, 0.f, -1.f,
        0.5f, -0.4330127019f, 0.0f,
        0.f, 0.f, -1.f,
        0.0f,  0.4330127019f, 0.0f
    };
    f32 mtest_triangle_colors[12] =
    {
        1.f, .0f, .0f, 1.f,
        .0f, 1.f, .0f, 1.f,
        .0f, .0f, 1.f, 1.f
    };


    // Teapot Articles
    mesh_id test_utah_teapot_id = -1;
    fmesh test_utah_teapot;
    fpath utah_teapot_file;
    std::vector<v3> test_utah_teapot_vertecies;
    std::vector<v4> test_utah_teapot_colors;

    shader_program_id test_shader = -1;
    shader_id test_shader_frag = -1;
    shader_id test_shader_vert = -1;
    fpath test_shader_frag_file = "test_utah_teapot.frag";
    fpath test_shader_vert_file = "test_utah_teapot.vert";


    frame_shader_global frame_data;
    frame_shader_global& frame_shader_globals = frame_data;

    public:
    renderer_backend platform = {};

    CONSTRUCTOR renderer();

    void frame_update();
};

}
