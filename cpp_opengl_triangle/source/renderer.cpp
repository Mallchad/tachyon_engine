
#include "renderer.h"

#include "include_tracy.h"

#include "code_helpers.h"
#include "math.hpp"
#include "file.hpp"


CONSTRUCTOR renderer::renderer()
{
    test_utah_teapot_vertecies = read_stl_file( "assets/utah_teapot.stl" );

    test_utah_teapot =
    {
        .name = "test_utah_teapot",
        .vertex_buffer = test_utah_teapot_vertecies.data(),
    };
    test_utah_teapot.vertex_count = test_utah_teapot_vertecies.size()  * 3;

    test_utah_teapot_id = platform.mesh_create( test_utah_teapot );
}

void
FUNCTION renderer::frame_update(ffloat epoch_elapsed)
{
    ZoneScopedN("graphics refresh");

    platform.frame_start();
    ftransform stub_transform = {};
    platform.draw_mesh( test_utah_teapot_id, stub_transform, 0 );
    (void)(epoch_elapsed);
    platform.refresh();
}
