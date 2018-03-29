#version 400

// this demo is based on code from here https://learnopengl.com/#!PBR/Lighting
/// @brief the vertex passed in
layout (location = 0) in vec3 inVert;
/// @brief the normal passed in
layout (location = 2) in vec3 inNormal;
/// @brief the in uv
layout (location = 1) in vec2 inUV;

out vec3 v_position;
out vec3 v_normal;
out vec2 v_uv;

uniform mat4 MVP;
uniform mat4 N;
uniform mat4 M;

void main()
{
  v_position = inVert;
  v_normal = inNormal;
  v_uv = inUV;
}
