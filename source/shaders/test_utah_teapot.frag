#version 330 core

smooth in vec4 vertex_color;
out vec4 frag_color;

void main()
{
    vec4 fc = gl_FragCoord;
    frag_color = fc;
}
