#version 410 core

layout (location = 0) in vec3 in_vert;

out vec3 vs_localPos;

uniform mat4 u_MV;
uniform mat4 u_P;

void main()
{
    vs_localPos = in_vert;
    gl_Position =  u_P * u_MV * vec4(in_vert, 1.0);
}
