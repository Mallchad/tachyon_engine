#version 420 core

layout (location = 0) in vec3 normal;
layout (location = 1) in vec3 vert;
layout (location = 2) in vec4 col;

out vec3 v_normal;
out vec3 v_position;
smooth out vec4 vertex_color;


const float tau = 6.283185307;

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

const float scale = .1;
const float ratio_16_9 = 1080.f/1920.f;

float rotation_speed = 0.1f;
float x_rot = (-0.125f)  * tau;
float y_rot = (0. + (time_since_epoch * rotation_speed)) * tau;
float z_rot = (0.0f) *tau;

// Translation
vec3 trans = vec3( 0.0, 0.0, 0.0);

void main()
{
    vec4 vertex = vec4( vert.x, vert.y, vert.z, 1.0 );

    mat4 transform = mat4( 1., .0, .0, 0,
                           .0, 1., .0, 0,
                           .0, .0, 1., 0,
                           .0, .0, .0, 1. );
    mat4 rotation = mat4(1.,     0.,          0.,           0.,
                         0.,     cos(x_rot), -sin(x_rot),   0.,
                         0.,     sin(x_rot) , cos(x_rot),   0.,
                         0.,     0.,          0.,           1.);
    rotation *= mat4(cos(y_rot),    0.,     sin(y_rot),     0.,
                     0.,            1.,     0.,             0.,
                     -sin(y_rot),   0.,     cos(y_rot),     0.,
                     0.,            0.,    0.,              1.);

    rotation *= mat4(cos(z_rot),    -sin(z_rot),    0.,     0.,
                     sin(z_rot),    cos(z_rot),     0.,     0.,
                      0.,            0.,             1.,     0.,
                     0.,            0.,             0.,     1.);

    transform *= rotation;
    transform *= mat4(scale,     0.,    0.,    trans.x,
                      0.,     scale,    0.,    trans.y,
                      0.,        0., scale,    trans.z,
                      0.,        0.,    0.,    1.);


    // // Map into screen coordinate system
    vertex *= transform;
    vertex.x *= screen_vh_aspect_ratio;

    gl_Position = vertex;



    v_normal = ( vec4( normal, 0.0 ) * rotation ).xyz;
    v_position = (vertex * transform).xyz;
    vertex_color = col;
}
