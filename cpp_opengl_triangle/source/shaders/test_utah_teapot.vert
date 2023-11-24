#version 330 core

layout (location = 0) in vec3 vert;
layout (location = 2) in vec4 col;
smooth out vec4 vertex_color;


const float tau = 6.283185307;
const float x_rotation = (-0.25f + 1.f)  *tau;
const float y_rotation = (0.0 + 1.f) *tau;
const float z_rotation = (0.0f + 1.f) *tau;
const float x_rot = (-0.125f + 1.f)  *tau;
const float y_rot = (0.0 + 1.f) *tau;
const float z_rot = (0.0f + 1.f) *tau;
const vec4 arbitrary_axis = vec4( 0.662f, 0.2f, 0.722f, 1.f);

const float scale = 0.05f;
const float ratio_16_9 = 1080.f/1920.f;

void main()
{
    vec4 transform = vec4( vert.x, vert.y, vert.z, 1.f/ scale );
    mat4 test = mat4(1.,     0.,          0.,           0.,
                     0.,     cos(x_rot), -sin(x_rot),   0.,
                     0.,     -sin(x_rot), cos(x_rot),   0.,
                     0.,     0.,          0.,           1.);
    vec4 rotation = transform * test;
    rotation *= mat4(cos(y_rot),    0.,     sin(y_rot),     0.,
                     0.,            1.,     0.,             0.,
                     -sin(y_rot),  0.,     cos(y_rot),     0.,
                     0.,            0.,    0.,              1.);
    // transform.y = (cos(x_rotation) * arbitrary_axis.y) - (sin(x_rotation)* arbitrary_axis.z);
    // transform.z = (sin(x_rotation) * arbitrary_axis.y) + (cos(x_rotation)* arbitrary_axis.z);

    // transform.x = (cos(y_rotation) * arbitrary_axis.x) + (sin(y_rotation)* arbitrary_axis.z);
    // transform.z = (-sin(y_rotation) * arbitrary_axis.x) + (cos(y_rotation)* arbitrary_axis.z);

    // transform.x = (cos(z_rotation) * arbitrary_axis.x) - (sin(z_rotation)* arbitrary_axis.y);
    // transform.y = (sin(z_rotation) * arbitrary_axis.x) + (cos(z_rotation)* arbitrary_axis.y);

    transform.y = (cos(x_rotation) * transform.y) - (sin(x_rotation)* transform.z);
    transform.z = (sin(x_rotation) * transform.y) + (cos(x_rotation)* transform.z);

    transform.x = (cos(y_rotation) * transform.x) + (sin(y_rotation)* transform.z);
    transform.z = (-sin(y_rotation) * transform.x) + (cos(y_rotation)* transform.z);

    transform.x = (cos(z_rotation) * transform.x) - (sin(z_rotation)* transform.y);
    transform.y = (sin(z_rotation) * transform.x) + (cos(z_rotation)* transform.y);

    // // Map into screen aspect
    transform.x *= ratio_16_9;
    rotation.x *= ratio_16_9;
    gl_Position = rotation;
    vertex_color = col;
}
