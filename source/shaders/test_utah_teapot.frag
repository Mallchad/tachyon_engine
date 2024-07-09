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

struct material
{
    vec3 color;
    float specular;
    float roughness;
    float metallic;
    float opacity;
    vec3 emissive;
    vec3 normal;
};

void main()
{
    material metal_polished;

    metal_polished.color = v_color.xyz;
    metal_polished.specular = 1.0;
    metal_polished.roughness = 0.1;
    metal_polished.metallic = 1.0;
    metal_polished.opacity = v_color.w;
    metal_polished.emissive = vec3( 0.0 );
    metal_polished.normal = v_normal;

    // Polished plastic
    material plastic;

    plastic.color = v_color.xyz;
    plastic.specular = 1.0;
    plastic.roughness = 0.5;
    plastic.metallic = 0.0;
    plastic.opacity = v_color.w;
    plastic.emissive = vec3( 0.0 );
    plastic.normal = v_normal;

    material m = metal_polished;

    // This is current done entirely in NDC (Normalized Device Coordinates)
    vec4 camera_pos = vec4( 0.0, 0.0, 0.0, 0.0 );
    vec3 point_light = vec3( 2.0, 5.0, 0.0 );
    float light_intensity = 5.0;
    vec3 light_color = vec3( 1.0, 1.0, 1.0 ) * light_intensity;

    vec3 diff = m.color;
    vec3 spec = vec3( clamp( m.specular, 0.0, 1.0 ) );
    m.roughness = clamp( m.roughness, 0.0, 1.0 );
    float opac = m.opacity;

    float ambient = 0.0;
    // Fragment Coordinate
    // Really frustrating to convert to screen/worldspace, use vertecies instead
    vec3 frag = v_position;


    m.metallic = clamp( 1-m.metallic, 0.0, 1.0 );

    // Point Light
    vec3 point_ray = normalize( point_light - frag );
    // Directional Right
    vec3 directional_ray = normalize( point_light - vec3(0.0) );

    vec3 view_ray = normalize( frag - camera_pos.xyz );
    vec3 reflect_ray = reflect( -point_ray, v_normal );
    // Max and clamp to prevent negative colors
    // Divide light contribution by 3 to prevent applying contribution 3 times
    float light_contribution = max(dot( -point_ray, v_normal ), 0.0 ) * 1.0;
    m.roughness = clamp( 1-m.roughness, 0.0, 1.0 );
    float reflect_contribution = pow( max( dot( view_ray, reflect_ray ) , 0.0 ), 1/m.specular );
    vec3 diffuse = ( light_contribution * light_color );
    spec *= ( reflect_contribution  ) * light_color;
    m.metallic = clamp( 1-m.metallic, 0.0, 1.0 );

    vec3 metal = light_color * m.color * max( pow( dot( view_ray, reflect_ray ),
                                                   8*(m.roughness) ), 0.0 );
    vec3 reflect = mix( spec, metal, m.metallic) * light_contribution;
    diffuse = mix( diffuse, vec3(0.0), (m.roughness) );

    vec3 total = ( (ambient + diffuse) * m.color ) + reflect;
    frag_color = vec4( total , 1.0 );

    // Uncomment for debug colours
    // frag_color = vec4( vec3(light_contribution), opac );   // Diffuse Reflection
    // frag_color = vec4( vec3(spec), opac );   // Specular Reflection
    // frag_color = vec4( vec3(metal), opac );   // Metallic Reflection
    // frag_color = vec4( vec3(reflect), opac );   // Metallic Reflection
    // frag_color = vec4( m.color, 1.0 );  // Base Color
    // frag_color = vec4( v_position.xyz, 1.0 );  // Frag Colours

    // frag_color = vec4( v_normal.xyz, 1.0 );  // Normal
    // frag_color = vec4( 0.7, 0.7, 0.7, 1.0 );                // Gray
}
