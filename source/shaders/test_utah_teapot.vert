#version 420 core

layout (location = 0) in vec3 vert;
layout (location = 2) in vec4 col;
smooth out vec4 vertex_color;


const float tau = 6.283185307;
const float x_rotation = (-0.25f + 1.f)  *tau;
const float y_rotation = (0.0 + 1.f) *tau;
const float z_rotation = (0.0f + 1.f) *tau;

layout(std140) uniform frame_data
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

const vec4 arbitrary_axis = vec4( 0.662f, 0.2f, 0.722f, 1.f);

const float scale = 0.05f;
const float ratio_16_9 = 1080.f/1920.f;

float rotation_speed = 0.1f;
float x_rot = (-0.25f)  * tau;
float y_rot = (0. + (time_since_epoch * rotation_speed)) * tau;
float z_rot = (0.0f) *tau;

void main()
{
    vec4 transform = vec4( vert.x, vert.y, vert.z, 1.f/ scale );
    mat4 test = mat4(1.,     0.,          0.,           0.,
                     0.,     cos(x_rot), -sin(x_rot),   0.,
                     0.,     sin(x_rot) , cos(x_rot),   0.,
                     0.,     0.,          0.,           1.);
    vec4 rotation = transform * test;
    rotation *= mat4(cos(y_rot),    0.,     sin(y_rot),     0.,
                     0.,            1.,     0.,             0.,
                     -sin(y_rot),   0.,     cos(y_rot),     0.,
                     0.,            0.,    0.,              1.);

    rotation *= mat4(cos(z_rot),    -sin(z_rot),    0.,     0.,
                     sin(z_rot),    cos(z_rot),     0.,     0.,
                     0.,            0.,             1.,     0.,
                     0.,            0.,             0.,     1.);



    rotation *= mat4( 1., .0, .0, 0,
                      .0, 1., .0, 0,
                      .0, .0, 1., 0,
                      .0, .0, .0, 1. );

    // // Map into screen aspect
    transform.x *= ratio_16_9;
    rotation.x *= ratio_16_9;
    gl_Position = rotation;
    vertex_color = col;
}
