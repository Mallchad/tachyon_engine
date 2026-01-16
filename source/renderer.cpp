
#include "include_core.h"

namespace tyon
{

CONSTRUCTOR renderer::renderer()
{
    // Generic initialization
    global = globals::get_primary();
    frame_shader_globals.last_begin_epoch = time_elapsed<f32>();
    frame_shader_globals.last_end_epoch = time_elapsed<f32>();

    utah_teapot_file = linux_search_file(
        "utah_teapot.stl", { std::filesystem::path( global->project_root ) / "assets" } );
        // "articulated_whale_shark.stl", { std::filesystem::path( global->project_root ) / "assets" } );
    test_utah_teapot = read_stl_file( utah_teapot_file );

    bool file_read_fail = test_utah_teapot.vertex_count <= 0;
    if ( file_read_fail ) { return; }

    test_utah_teapot.name = "test_utah_teapot";

    test_utah_teapot.vertex_color_buffer.resize( test_utah_teapot.vertex_count );
    v4 teapot_color =  { 0.3f, .7f, 0.3f , 1.f };
    test_utah_teapot.vertex_color_buffer.assign( test_utah_teapot.vertex_count, teapot_color );

    test_utah_teapot.vertex_color_buffer = test_utah_teapot_colors;
    test_utah_teapot.shader_id = -1;

    test_utah_teapot_id = platform.mesh_create( test_utah_teapot );

    // Platform specific initialization
    // Setup test articles
    shader_fragment_test = platform.shader_create( "shader_fragment_test", shader_type::fragment );
    shader_vertex_test = platform.shader_create( "shader_vertex_test", shader_type::vertex );

    fstring shadersource_triangle_fragment = linux_load_text_file( "test_triangle.frag",
                                                                   global->asset_search_paths );
    fstring shadersource_triangle_vertex = linux_load_text_file( "test_triangle.vert",
                                                                 global->asset_search_paths );
    platform.shader_compile( shader_fragment_test, shadersource_triangle_fragment );
    platform.shader_compile( shader_vertex_test, shadersource_triangle_vertex );

    shader_program_triangle = platform.shader_program_create( "shader_triangle_test" );
    platform.shader_program_attach( shader_program_triangle, shader_vertex_test );
    platform.shader_program_attach( shader_program_triangle, shader_fragment_test );
    platform.shader_program_compile( shader_program_triangle );

    fmesh test_triangle =
    {
        .name = "test_triangle",
        .face_count = 1,
        .vertex_count = 3,
        .color_count = 3,
        .shader_id = shader_program_triangle
    };
    test_triangle.vertex_buffer.resize( test_triangle.vertex_count * 2 );
    test_triangle.vertex_color_buffer.resize( test_triangle.color_count );
    std::memcpy( test_triangle.vertex_buffer.data(),
                 mtest_triangle,
                 sizeof(mtest_triangle) );
    std::memcpy( test_triangle.vertex_color_buffer.data(),
                 mtest_triangle_colors,
                 sizeof(mtest_triangle_colors) );
    mtest_triangle_mesh = platform.mesh_create( test_triangle );

    // Setup Scene Objects
    v3f a = {1, 2, 3};
    camera = matrix::one();
    frame_shader_globals.camera = camera;
}

void
FUNCTION renderer::frame_update()
{
    PROFILE_SCOPE_FUNCTION();

    cstring frame_label = "Graphical";
    FrameMarkNamed( frame_label );

    // Update early data for the frame
    frame_shader_globals.time_since_epoch = time_elapsed<f32>();
    frame_shader_globals.delta_time_begin =
        time_elapsed<f32>() - frame_shader_globals.last_begin_epoch;
    frame_shader_globals.last_begin_epoch = time_elapsed<f32>();
    frame_shader_globals.delta_time = frame_shader_globals.delta_time_begin;

    if (global->reload_shaders)
    {
        global->reload_shaders = false;

        fstring new_shader_source( linux_load_text_file( test_shader_frag_file,
                               { global->project_root / "source/shaders" } ) );
        fstring vert_shader_source( linux_load_text_file( test_shader_vert_file,
                               { global->project_root / "source/shaders" } ) );
        shader_program_id volatile_shader = -1;
        shader_program_id tmp_shader = 0;

        test_shader_frag = platform.shader_create( "utah_teapot", shader_type::fragment );
        test_shader_vert = platform.shader_create( "utah_teapot", shader_type::vertex );
        platform.shader_compile( test_shader_frag, new_shader_source );
        platform.shader_compile( test_shader_vert, vert_shader_source );
        volatile_shader = platform.shader_program_create( "new",
                                                          { test_shader_frag,
                                                         test_shader_vert } );
        if ( platform.shader_program_compile( volatile_shader ))
        {
            platform.shader_program_destroy( test_shader );
            test_shader = volatile_shader;
            platform.shader_program_compile( test_shader );
            std::cout << "[Renderer] Reloaded shader test_utah_teapot \n";
        }
        else
        {
            std::cout << "[Renderer] ! Shader reload failed? \n";
        }
    }

    v3 velocity;
    velocity = velocity * 3.0f;

    // Random temporary stuff
    v3 left_vector = { -1., .0, .0 };
    f32 camera_speed = 45.0 * frame_data.delta_time;

    f32 tau = 6.283185307;
    transform camera_transform;
    memory_zero( &camera_transform, 1 );
    camera_transform.rotation.x = tau * 0.25;

    if (global->action_left)
    { camera_transform.translation += v4 {v3::left()} *
                                         matrix::create_rotation( camera_transform.rotation ) *
                                         camera_speed; }
    if (global->action_right)
    { camera_transform.translation += v4(v3::right()) *
                                         matrix::create_rotation( camera_transform.rotation ) *
                                         camera_speed; }
    if (global->action_forward)
    { camera_transform.translation += v4(v3::forward()) *
                                         matrix::create_rotation( camera_transform.rotation ) *
                                         camera_speed; }
    if (global->action_backward)
    { camera_transform.translation += v4(v3::backward()) *
                                         matrix::create_rotation( camera_transform.rotation ) *
                                         camera_speed; }
    if (global->action_up)
    { camera_transform.translation += v4(v3::up()) *
                                         matrix::create_rotation( camera_transform.rotation ) *
                                         camera_speed; }
    if (global->action_down)
    { camera_transform.translation += v4(v3::down()) *
                                         matrix::create_rotation( camera_transform.rotation ) *
                                         camera_speed; }
    v3 camera_rotation = { 0., tau, 0 };
    frame_data.camera *= (camera_transform.translation_matrix() );

    uniform uniform_frame;
    uniform_frame.pack( frame_shader_globals.epoch,
                        frame_shader_globals.time_since_epoch,
                        frame_shader_globals.last_begin_epoch,
                        frame_shader_globals.last_end_epoch,
                        frame_shader_globals.delta_time,
                        frame_shader_globals.delta_time_begin,
                        frame_shader_globals.delta_time_end,
                        frame_shader_globals.screen_vh_aspect_ratio,
                        frame_data.camera.unreal_to_opengl().transpose() );
    platform.shader_globals_update( uniform_frame );
    platform.frame_start();
    ftransform stub_transform = {};
    platform.draw_mesh( test_utah_teapot_id, stub_transform, test_shader );
    platform.draw_mesh( mtest_triangle_mesh, stub_transform, shader_program_triangle );
    // platform.draw_test_signfield( platform.msignfield_color );

    platform.refresh( frame_shader_globals );

    std::cout << std::flush;
    // fflush( stdout );

    frame_shader_globals.delta_time_end =
        time_elapsed<f32>() - frame_shader_globals.last_end_epoch;
    frame_shader_globals.last_end_epoch = time_elapsed<f32>();
    FrameMarkEnd( frame_label );
}

}
