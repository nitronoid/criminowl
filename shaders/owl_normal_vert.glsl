#version 410 core

layout (location = 0) in vec3 inVert;
layout (location = 1) in vec2 inUV;

out vec2 TexCoords;

void main()
{
  TexCoords = inUV;
  gl_Position = vec4(inVert, 1.0);
}
