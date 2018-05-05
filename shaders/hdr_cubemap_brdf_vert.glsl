#version 410 core

layout (location = 0) in vec3 in_vert;
layout (location = 1) in vec2 in_uv;

out vec2 vs_texCoords;

void main()
{
  vs_texCoords = in_uv;
  gl_Position = vec4(in_vert, 1.0);
}
