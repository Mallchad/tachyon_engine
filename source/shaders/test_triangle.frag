#version 440 core

layout (location = 0) smooth in vec4 vertex_color;
layout (location = 0) out vec4 frag_color;

void main()
{
    vec4 fc = gl_FragCoord;

    frag_color = vertex_color;
}
