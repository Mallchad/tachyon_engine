
#include "renderer.h"

#include "include_tracy.h"

#include "code_helpers.h"
#include "math.hpp"
#include "file.hpp"


CONSTRUCTOR renderer::renderer()
{
    global = global_database::get_primary();

    utah_teapot_file = linux_search_file(
        "utah_teapot.stl", { std::filesystem::path( global->project_root ) / "assets" } );
    test_utah_teapot_vertecies = read_stl_file( utah_teapot_file );

    bool file_read_fail = test_utah_teapot_vertecies.size() <= 0;
    if ( file_read_fail ) { return; }

    test_utah_teapot_colors.resize( test_utah_teapot_vertecies.size() );
    ffloat4 teapot_color =  { 0.3f, .7f, 0.3f , 1.f };
    test_utah_teapot_colors.assign( test_utah_teapot_colors.size(), teapot_color );

    test_utah_teapot =
    {
        .name = "test_utah_teapot",
        .vertex_buffer = test_utah_teapot_vertecies.data(),
        .vertex_color_buffer = test_utah_teapot_colors.data(),
        .shader_program_id = -1
    };
    test_utah_teapot.vertex_count = test_utah_teapot_vertecies.size();
    test_utah_teapot.color_count = test_utah_teapot_vertecies.size();

    test_utah_teapot_id = platform.mesh_create( test_utah_teapot );


}

void
FUNCTION renderer::frame_update(ffloat epoch_elapsed)
{
    ZoneScopedN("graphics refresh");

    if (global->reload_shaders)
    {
        global->reload_shaders = false;
        fpath found_fragment_file =
            linux_search_file( test_shader_frag_file,
                               { global->project_root / "source/shaders" } );
        fpath found_vertex_file =
            linux_search_file( test_shader_vert_file,
                               { global->project_root / "source/shaders" } );

        byte_buffer new_shader = intern_file( found_fragment_file );
        byte_buffer vert_shader = intern_file( found_vertex_file );

        fstring new_shader_source( reinterpret_cast<const char*>( new_shader.data() ),
                                        new_shader.size() );
        fstring vert_shader_source( reinterpret_cast<const char*>( vert_shader.data() ),
                                         vert_shader.size() );
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

    platform.frame_start();
    ftransform stub_transform = {};
    platform.draw_mesh( test_utah_teapot_id, stub_transform, test_shader );
    (void)(epoch_elapsed);
    platform.refresh( frame_shader_globals );
    std::cout << std::flush;
}
