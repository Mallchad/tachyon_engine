#version 430 core

// Vertex Info
layout( location = 0 ) in vec3 v_normal;
layout( location = 1 ) in vec3 v_position;
layout( location = 2 ) smooth in vec4 v_color;

layout( location = 0 ) out vec4 frag_color;

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
    vec4 camera_pos = vec4( 0.0, 0.0, 0.0, 0.0 );
    vec3 point_light = vec3( 0.0, 5.0, -5.0 );
    float light_intensity = 1.0;
    vec3 light_color = vec3( 1.0, 1.0, 1.0 ) * light_intensity;
    vec3 specular = vec3( 0.3 );
    float roughness = 0.4;
    float ambient = 0.0f;
    vec3 frag = gl_FragCoord.xyz;
    float metallicness = 0.5;

    vec3 light_ray = normalize( point_light - vec3(0.0) );
    vec3 view_dir = normalize( gl_FragCoord.xyz - camera_pos.xyz );
    vec3 reflection_dir = reflect( -light_ray, v_normal );
    // Max and clamp to prevent negative colors
    // Divide light contribution by 3 to prevent applying contribution 3 times
    float light_contribution = max(dot( light_ray, v_normal ), 0.0 ) * 1.0;
    roughness = clamp( 1-roughness, 0.0, 1.0 );
    float specular_contribution = pow( max( dot( view_dir, reflection_dir ), 0.0 ), 1/ roughness );
    vec3 diffuse = (light_contribution * light_color * v_color.xyz );
    specular *= ( light_contribution * specular_contribution * light_color ) ;
    metallicness = clamp( 1-metallicness, 0.0, 1.0 );
    vec3 metallic = max( light_contribution * v_color.xyz,
                         metallicness) ;

    frag_color = vec4( (ambient + diffuse + specular) * (metallic) , 1.0 );
}
