
#pragma once

namespace tyon
{

enum class e_window_platform : i32
{
    none = 0,
    any = 1,
    x11 = 2,
    wayland = 3,
    windows = 4
};

struct render_context
{
    // Primary window size
    v2 window_size = { 1920.0f, 1080.0f };
    e_window_platform window_platform = e_window_platform::none;
    bool renderdoc_attached = false;
    bool nsight_attached = false;
};

struct mesh
{
    uid id;
    fstring name;
    array<v3> vertexes;
    array<v3> vertex_normals;
    array<i32> vertex_indexes;
    array<v4> vertex_colors;

    // These are used as statistics/convenience variables

    // Number of faces
    i32 faces_n = 0;
    // Number of vertecies
    i32 vertexes_n = 0;
    // Number of vertex indices
    i32 vertex_indexes_n = 0;
};

#pragma pack(push, 1)
struct frame_general_uniform
{
    // Timestamp of the very beginning of the program lifetime
    f32 epoch = 0;
    // Time elapsed since program epoch
    f32 time_since_epoch = 0;
    /// Time since epoch at the beginning of previous frame
    f32 last_begin_epoch = 0;
    /// Time since epoch at end of previous frame
    f32 last_end_epoch = 0;
    /// Time between last frame and current frame measured at unspecified time during frame
    f32 delta_time = 0;
    /// Time between last frame and current frame measured at beginning of each frame
    f32 delta_time_begin = 0;
    /// Time between last frame and current frame measured at beginning of each frame
    f32 delta_time_end = 0;
    // Screen aspect ratio given as vertical over horizontal
    f32 screen_vh_aspect_ratio = 1080.f/1920.f;
    // 32nd byte here. Already aligned, no padding required
    // Primary activate camera
    matrix camera;
};
#pragma pack(pop)

extern render_context* g_render;

PROC render_init() -> void;

PROC render_tick() -> void;

PROC mesh_init( mesh* arg ) -> fresult;

}
