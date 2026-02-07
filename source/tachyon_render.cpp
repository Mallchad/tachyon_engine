
#include "include_core.h"

namespace tyon
{

render_context* g_render = nullptr;

auto sdl = tyon::sdl_platform_procs_create();

PROC render_init() -> void
{
    PROFILE_SCOPE_FUNCTION();

    g_render = memory_allocate<render_context>( 1 );

    /* If nsight or renderdoc is attached it will break with wayland, so we
       should try to disable pre-emptively that if possible. */
    if (REFLECTION_PLATFORM_LINUX)
    {   char* renderdoc_env = std::getenv( "ENABLE_VULKAN_RENDERDOC_CAPTURE" );
        if (renderdoc_env && renderdoc_env[0] == '1')
        {   TYON_LOG( "Renderdoc is attached and doesn't support Wayland, falli3ng back to X11" );
            g_render->renderdoc_attached = true;
        }
    }

    cmdline_argument window_arg = g_library->cmdline_arguments.linear_search(
        []( cmdline_argument& arg ) {
        return arg.name == "window_platform"; }).copy_default( {} );
    fstring window_platform = window_arg.value.get_string();
    if (window_platform.size())
    {   TYON_LOGF( "Selected window_platform arg '{}'", window_platform );
        if (window_platform == "x11")
        {   g_render->window_platform = e_window_platform::x11;
        }
        else if (window_platform == "wayland")
        {   g_render->window_platform = e_window_platform::wayland;
        }
        else if (window_platform == "windows")
        {   g_render->window_platform = e_window_platform::windows;
        }
        else
        {   TYON_ERROR( "window_platform specified on command line does not match known platform" );
        }
    }
    // SDL needs to setup after the render context but before vulkan init
    sdl.init();

    tyon::window default_window = {
        .name = "VMEC | Spectral Renderer",
        .size = tyon::v2 { 1920.0f, 1000.0f },
        .position = tyon::v2 { 0.0f, 0.0f },
        .maximized = true
    };
    sdl.window_open( &default_window );

    /* This "camera" is for UI usage... So the sensor size should be the window
     * size, and all the primitives exactly on the near clip or slightly
     * behind. */
    g_render->main_camera = scene_camera {
        // Good size for UI, needs to be changed though...
        .sensor_size = default_window.size,
        .far_clip = 2.0f,
        .near_clip = 1.0f,
        // Move the camera back to allow -1 to 1 layer depths
        .transform { .translation = { 0.0, -1, 0.0  }}
    };

    bool vulkan_ok = false;
    bool opengl_ok = false;

    switch (global->render_backend)
    {
        case e_render_backend::vulkan:
            x11_init();
            x11_window_open();
            vulkan_ok = vulkan_init();
            break;
        case e_render_backend::opengl:
            // opengl_ok = opengl_init(); break;
            TYON_BREAK();
            break;
        default: break;

    }
}

PROC render_tick() -> void
{
    PROFILE_SCOPE_FUNCTION();

    sdl.tick();
    switch (global->render_backend)
    {
        case e_render_backend::vulkan:
            vulkan_tick();
            break;
        case e_render_backend::opengl:
            TYON_BREAK();
            break;
        default: break;
    }
}

PROC mesh_init( mesh* arg ) -> fresult
{
    if (arg->id.valid())
    {   VULKAN_ERRORF( "Tried to initialize already initialize mesh '{}'", arg->name );
        return false;
    }
    if (arg->vertexes.size() <= 0)
    {   VULKAN_ERRORF( "Tried to initialize mesh '{}' with {} vertexes",
                       arg->name, arg->vertexes.size() * 3 );
        return false;
    }

    arg->id = uuid_generate();
    arg->faces_n = arg->vertexes.size() / 3;
    arg->vertexes_n = arg->vertexes.size();
    arg->vertex_indexes_n = arg->vertex_indexes.size();

    TYON_LOG( "Created mesh: ")
    TYON_LOGF( "    ID: {}", arg->id );
    TYON_LOGF( "    Name: {}", arg->name );
    TYON_LOGF( "    Faces: {}", arg->faces_n );
    TYON_LOGF( "    Vertexes: {}", arg->vertexes_n );
    TYON_LOGF( "    Vertex Indexes: {}", arg->vertex_indexes_n );
    return true;
}


matrix scene_camera::create_perspective_projection()
{
    f32 camera_width = std::round( sensor_size.x );
    f32 aspect_ratio_vh = sensor_size.y / sensor_size.x;
    // Clip Plane Dimensions
    // Near
    f32 n = this->near_clip;
    // Far
    f32 f = this->far_clip;
    // Right
    f32 r = -(camera_width / 2.0) / aspect_ratio_vh;
    // Left
    f32 l = -r;
    // Top
    f32 t = (camera_width / 2.0);
    // Bottom
    f32 b = -t;

    // TODO: Temporary fix cuz too lazy to transpose algorithm.
    // TODO:. Well... This is for NDC coordinates...
    matrix result = matrix{
        (2.f*n) / (r-l),       0.f,               0.0f,         0,
              0.f,        (2.f*n) / (t-b),        0.0f,         0,
        (r+l) / (r-l),    (t+b) / (t-b),    -(f+n) / (f-n),    -1,
              0.f,             0.f,      -(2.f*f*n) / (f-n),    0
    }.transpose();

    return result;
}

PROC scene_camera::create_orthographic_projection() -> matrix
{
    f32 camera_width = std::round( sensor_size.x );
    f32 aspect_ratio_vh = sensor_size.y / sensor_size.x;
    // Clip Plane Dimensions
    // Near
    f32 n = this->near_clip;
    // Far
    f32 f = this->far_clip;
    // Right
    f32 r = -(camera_width / 2.0) / aspect_ratio_vh;
    // Left
    f32 l = -r;
    // Top
    f32 t = (camera_width / 2.0);
    // Bottom
    f32 b = -t;

    // This is rearranged for z up, x right, y backward, Unreal/Tachyon coordinates
    matrix result = matrix{
        2/(r-l),    0.0f,       0.0f,       -(r+l) / (r-l),
        0.0f,       2/(f-n),    0.0f,       -(f+2) / (f-n),
        0.0f,       0.0f,       2/(t-b),    -(t+b)/(t-b),
        0.0f,       0.0f,       0.0f,       1.0f
    };

    return result;
}

}
