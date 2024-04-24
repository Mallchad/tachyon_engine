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

const vec3 arbitrary_axis = vec3( 0.662f, 0.2f, 0.722f );

const float scale = .1;
const float ratio_16_9 = 1080.f/1920.f;

const float tau = 6.283185307;

float rotation_speed = 0.1f;
// Euler Rotation
vec4 rot = vec4(0);

// Translation
vec3 trans = vec3( 0.0, 0.0, 0.0);

// Create a Euler Rotation Matrix with w component being around an arbitrary axis
mat4 create_rotation( vec4 euler )
{
    // Rotation amounts
    vec4 r = euler;
    vec3 a = arbitrary_axis;
    float ar = euler.w;
    a = normalize(a.xyz);

    // Rotation around an arbitrary axis defined by Euler coordinates
    mat4 arbitraty_matrix =
        mat4( cos(ar)+(a.x*a.x) * (1-cos(ar)),
              a.x*a.y*(1-cos(ar)) - a.z*sin(ar),
              a.x*a.z*(1-cos(ar)) + a.y*sin(ar),
              0,

              a.y*a.x *(1 - cos(ar)) + a.z*sin(ar),
              cos(ar) + (a.y*a.y)*(1 - cos(ar)),
              a.y*a.z*(1 - cos(ar)) - a.x*sin(ar),
              0,

              a.z*a.x* (1 - cos(ar)) - a.y*sin(ar),
              a.z*a.y* (1 - cos(ar)) + a.z*sin(ar),
              cos(ar) + (a.x*a.x) * (1 - cos(ar)),
              0,

              0, 0, 0, 1
              );

    // Math representation rotation matrix, needs to be rearranged to collumn major
    // [[cos(y) * cos(z), -cos(y) * sin(z), sin(y), 0],

    //  [cos(x) * sin(z) + cos(z) * sin(x) * sin(y),
    //  cos(x) * cos(z) - sin(x) * sin(y) * sin(z),
    //   -cos(y) * sin(x),
    //   0],

    //  [sin(x) * sin(z) - cos(x) * cos(z) * sin(y),
    //  cos(x) * sin(y) * sin(z) + cos(z) * sin(x),
    //   cos(x) * cos(y),
    //   0],

    //  [0, 0, 0, 1]]

    // // Euler Combined 3 Axis Rotation Matrix
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
    return rotation_matrix * arbitraty_matrix;
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
    mat4 projection = identity;             // Orthographic Camera to Clip-Space
    mat4 viewport = identity;               // Clip-Space to Screen Space

    rot.x = 0.0;
    rot.y = (0.0 + (time_since_epoch * rotation_speed)) * tau;
    rot.z = 0.0;
    rot.w = 0.0;

    vec4 vertex = vec4( vert.x, vert.y, vert.z, 1.0 );

    mat4 transform = identity;

    local *= create_rotation( vec4((1./8.)*tau, 0.0, 0.0, 0.0 ) );
    world *= create_rotation( rot );
    // Normal must be scaled and translated so it have to go first
    v_normal = (projection * camera * world * local * vec4( normal, 1.0 )).xyz;
    world *= mat4(scale,     0.,    0., 0.,
                  0.,     scale,    0., 0.,
                  0.,        0., scale, 0.,
                  trans.x, trans.y, trans.z,    1.);
    // Map into screen coordinate system
    projection[0][0] = screen_vh_aspect_ratio;
    transform = projection * camera * world * local * transform;

    // // Map into screen coordinate system
    vertex = projection * camera * world * local * vertex;
    gl_Position = vertex;

    v_position = vertex.xyz;
    v_color = vec4( 0.8, 0.0, 0.0 , 1.0 ) ;
    // v_color = vec4( 1.0 );
}
