#version 430 core

layout( location = 0 ) in vec3 normal;
layout( location = 1 ) in vec3 vert;
layout( location = 2 ) in vec4 col;

layout( location = 0 ) out vec3 v_normal;
layout( location = 1 ) out vec3 v_position;
layout( location = 2 ) smooth out vec4 v_color;
layout( location = 3 ) out mat4 world_matrix;


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
    // Primary activate camera
    mat4 camera;
};

const vec3 arbitrary_axis = vec3( 0.662f, 0.2f, 0.722f );

const float scale = 1;
// Translation
vec3 trans = vec3( 0.0, 50.0, 0.0 );

const float ratio_16_9 = 1080.f/1920.f;
const float tau = 6.283185307;

float rotation_speed = 0.1f;
// Euler Rotation
vec4 rot = vec4(0);

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
              a.y*a.x *(1 - cos(ar)) + a.z*sin(ar),
              a.z*a.x* (1 - cos(ar)) - a.y*sin(ar),
              0,

              a.x*a.y*(1-cos(ar)) - a.z*sin(ar),
              cos(ar) + (a.y*a.y)*(1 - cos(ar)),
              a.z*a.y* (1 - cos(ar)) + a.z*sin(ar),
              0,

              a.x*a.z*(1-cos(ar)) + a.y*sin(ar),
              a.y*a.z*(1 - cos(ar)) - a.x*sin(ar),
              cos(ar) + (a.x*a.x) * (1 - cos(ar)),
              0,

              0, 0, 0, 1 );


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

mat4 projection_create()
{
    float camera_width = 1.0;
    float aspect_ratio_vh = screen_vh_aspect_ratio;
    // Clip Plane Dimensions
    // Near
    float n = 1;
    // Far
    float f = 700.0;
    // Right
    float r = -(camera_width / 2.0) / aspect_ratio_vh;
    // Left
    float l = -r;
    // Top
    float t = (camera_width / 2.0);
    // Bottom
    float b = -t;

    mat4 out_projection =
        mat4( (2.*n) / (r-l),0.,                0.0,                0,
              0.,            (2.*n) / (t-b),    0.0,                0,
              (r+l) / (r-l), (t+b) / (t-b),     -(f+n) / (f-n),     -1,
              0.,            0.,                -(2.*f*n) / (f-n),  0 );

    return out_projection;
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
    mat4 cam = camera;                   // World to Camera Space
    mat4 projection = identity;             // Orthographic Camera to Clip-Space
    mat4 viewport = identity;               // Clip-Space to Screen Space
    // cam = mat4(1);                          // Debug

    // cam *= create_rotation( vec4(0., 0., 1., 0.) );

    rot.x = 0.0;
    rot.y = (0.0 + (time_since_epoch * rotation_speed)) * tau;
    // rot.y = 0.0;
    rot.z = 0.0;
    rot.w = 0.0;

    vec4 vertex = vec4( vert.x, vert.y, vert.z, 1.0 );

    mat4 transform = identity;

    local *= create_rotation( vec4((1./8.)*tau, 0.0, 0.0, 0.0 ) );
    mat4 world_anim = create_rotation( rot );
    world = mat4(scale,     0.,    0.,  0.,
                  0.,     scale,    0., 0.,
                  0.,        0., scale, 0.0,
                  trans.x, trans.y, trans.z,    1.);

    // Normals don't particularly need or want scale and translation so we're skipping some
    // Add back camera rotation when its made available
    vec4 norm = projection * world_anim * local * vec4( normal, 1.0 );

    // Perspective Projection
    // This is mapping into screen coordinate system and skewing based on distance
    projection = projection_create();

    vertex = projection * cam * world * world_anim * local * vertex;
    gl_Position = vertex;
    v_normal = normalize(norm).xyz;

    v_position = vertex.xyz;
    v_color = vec4( 0.8, 0.0, 0.0 , 1.0 ) ;
    world_matrix = projection * cam;

    // Debug Coloring
    // v_color = vec4( 1.0 );
}
