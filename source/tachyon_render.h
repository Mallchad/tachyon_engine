
#pragma once

struct render_context
{
    // Primary window size
    v2 window_size = { 1920.0f, 1080.0f };
};

struct mesh
{
    uid id;
    fstring name;
    array<v3> vertexes;
    array<i32> vertex_indexes;
    vector<v4> vertex_colors;

    // These are used as input parameters as well as convenience

    // Number of faces
    i32 faces_n = 0;
    // Number of vertecies
    i32 vertexes_n = 0;
    // Number of vertex indices
    i32 vertex_indexes_n = 0;
    i32 vertex_colors_n = 0;
};

extern render_context* g_render;

PROC render_init() -> void;

PROC render_tick() -> void;
