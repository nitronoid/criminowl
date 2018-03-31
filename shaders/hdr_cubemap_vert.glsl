#version 410 core

layout (location = 0) in vec3 inVert;

out vec3 localPos;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    localPos = inVert;  
    gl_Position =  projection * view * vec4(localPos, 1.0);
}
