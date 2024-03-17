
#include "renderer.h"

#include "include_tracy.h"

#include "code_helpers.h"
#include "math.hpp"
#include "file.hpp"


CONSTRUCTOR renderer::renderer()
{
    global = globals::get_primary();
    frame_shader_globals.last_begin_epoch = time_elapsed<ffloat>();
    frame_shader_globals.last_end_epoch = time_elapsed<ffloat>();

    utah_teapot_file = linux_search_file(
        "utah_teapot.stl", { std::filesystem::path( global->project_root ) / "assets" } );
    test_utah_teapot = read_stl_file( utah_teapot_file );

    bool file_read_fail = test_utah_teapot.vertex_count <= 0;
    if ( file_read_fail ) { return; }

    test_utah_teapot.name = "test_utah_teapot";

    test_utah_teapot.vertex_color_buffer.resize( test_utah_teapot.vertex_count );
    ffloat4 teapot_color =  { 0.3f, .7f, 0.3f , 1.f };
    test_utah_teapot.vertex_color_buffer.assign( test_utah_teapot.vertex_count, teapot_color );

    test_utah_teapot.vertex_color_buffer = test_utah_teapot_colors;
    test_utah_teapot.shader_program_id = -1;

    test_utah_teapot_id = platform.mesh_create( test_utah_teapot );


}

void
FUNCTION renderer::frame_update(ffloat epoch_elapsed)
{
    ZoneScopedN("graphics refresh");

    // Update early data for the frame
    frame_shader_globals.time_since_epoch = time_elapsed<ffloat>();
    frame_shader_globals.delta_time_begin =
        time_elapsed<ffloat>() - frame_shader_globals.last_begin_epoch;
    frame_shader_globals.last_begin_epoch = time_elapsed<ffloat>();
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

    platform.shader_globals_update( frame_shader_globals );
    platform.frame_start();
    ftransform stub_transform = {};
    platform.draw_mesh( test_utah_teapot_id, stub_transform, test_shader );

    platform.refresh( frame_shader_globals );

    std::cout << std::flush;

    frame_shader_globals.delta_time_end =
        time_elapsed<ffloat>() - frame_shader_globals.last_end_epoch;
    frame_shader_globals.last_end_epoch = time_elapsed<ffloat>();
}
