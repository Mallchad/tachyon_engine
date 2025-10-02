#version 440 core

layout (location = 0) in vec3 norm;
layout (location = 1) in vec3 vert;
layout (location = 2) in vec4 col;

layout( location = 0 ) out vec4 vertex_color;

layout(std140, binding = 0) uniform frame_data
{
    float epoch;
    float time_since_epoch;
    /// Time since epoch at the beginning of previous frame
    float last_begin_epoch;
    /// Time since epoch at end of previous frame
    float last_end_epoch;
    /// Time between last frame and current frame measured at unspecified time during frame
    float delta_time;
    /// Time between last frame and current frame measured at beginning of each frame
    float delta_time_begin;
    /// Time between last frame and current frame measured at beginning of each frame
    float delta_time_end;
    // Screen aspect ratio given as vertical over horizontal
    float screen_vh_aspect_ratio;
};

void main()
{
    float aspect_ratio = 0.5625;
    gl_Position = vec4(vert.x * aspect_ratio,
                       -vert.y,
                       vert.z,
                       1.f);
    vertex_color = col;

    // Debug
    // gl_Position = vec4(0.0);
}
