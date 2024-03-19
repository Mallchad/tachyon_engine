#version 330 core

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

in vec3 v_normal;
in vec3 v_position;
smooth in vec4 vertex_color;
out vec4 frag_color;

void main()
{

    vec3 directional_light = vec3( 2.0, 3.0, 1.0 );
    vec3 light_ray = normalize( directional_light - v_position );
    float light_intensity = 0.8;
    vec3 light_color = vec3( 1., 1., 1. ) * light_intensity;

    float diffuse_contribution = max(dot( light_ray, v_normal ), 0.0 );
    vec3 diffuse = diffuse_contribution * light_color;
    float ambient = 0.00f;
    vec3 fc = diffuse + ambient;

    frag_color = vec4( fc, 1.0 );
}
