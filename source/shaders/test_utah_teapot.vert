#version 430 core

layout( location = 0 ) in vec3 normal;
layout( location = 1 ) in vec3 vert;
layout( location = 2 ) in vec4 col;

layout( location = 0 ) out vec3 v_normal;
layout( location = 1 ) out vec3 v_position;
layout( location = 2 ) smooth out vec4 v_color;


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

const vec4 arbitrary_axis = vec4( 0.662f, 0.2f, 0.722f, 1.f);

const float scale = .1;
const float ratio_16_9 = 1080.f/1920.f;

const float tau = 6.283185307;

float rotation_speed = 0.1f;
// Euler Rotation
vec4 rot = vec4(0);

// Translation
vec3 trans = vec3( 0.0, 0.0, 0.0);

mat4 create_rotation( vec4 euler )
{
    // Rotation amounts
    vec4 r = euler;
    // Arbitrary axis direction to reduce gimbal lock
    vec4 a = arbitrary_axis;
    // Arbitrary axis of rotation
    // mat4 arbitraty_matrix = mat4( cos(r.y)+(a.x*a.x) * (1-cos( r.z), a.x*a.y*(1-cos) )
                                  // );

    // Combined 3 Axis Rotation Matrix
    mat4 rotation_matrix =
            // Row 1
        mat4( cos(r.y)*cos(r.z),
              -cos(r.y)*sin(r.z),
              sin(r.y),
              0,
              // Row 2
              cos(r.x)*sin(r.z) + cos(r.z)*sin(r.x)*sin(r.y),
              cos(r.x)*cos(r.z) - sin(r.x)*sin(r.y)*sin(r.z),
              -cos(r.y)*sin(r.x),
              0,
              // Row 3
              sin(r.x)*sin(r.z) - cos(r.x)*cos(r.z)*sin(r.y),
              cos(r.x)*sin(r.y)*sin(r.z) + cos(r.z)*sin(r.x),
              cos(r.x)*cos(r.y),
              0,
              // Row 4
              0, 0, 0, 1);
    return rotation_matrix;
}

void main()
{
    mat4 identity = mat4( 1., .0, .0, 0,
                          .0, 1., .0, 0,
                          .0, .0, 1., 0,
                          .0, .0, .0, 1. );
    // Transformation Matrices
    mat4 local = identity;                  // Local Space
    mat4 world = identity;                  // Local to World Space
    mat4 camera = identity;                 // World to Camera Space
    mat4 projection = identity;             // Orthographic to Clip-Space

    rot.x= (0.0) * tau;
    rot.y = (0.0 + (time_since_epoch * rotation_speed)) * tau;
    rot.z = (0.0) *tau;
    rot.w = 1.0;

    vec4 vertex = vec4( vert.x, vert.y, vert.z, 1.0 );

    mat4 transform = identity;

    local *= create_rotation( rot );
    world *= create_rotation( vec4((-3./8.)*tau, 0.0, 0.0, 1.0) );
    transform = projection * camera * world * local * transform;
    transform *= mat4(scale,     0.,    0.,    trans.x,
                      0.,     scale,    0.,    trans.y,
                      0.,        0., scale,    trans.z,
                      0.,        0.,    0.,    1.);

    // // Map into screen coordinate system
    vertex *= transform;
    vertex.x *= screen_vh_aspect_ratio;

    gl_Position = vertex;

    v_normal = (vec4( normal, 0.0 ) * projection * camera * world * local).xyz;
    v_position = vertex.xyz;
    v_color = vec4( 0.8, 0.0, 0.0 , 1.0 ) ;
    // v_color = vec4( 1.0 );
}
