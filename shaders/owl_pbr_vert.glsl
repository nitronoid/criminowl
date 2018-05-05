#version 400

// this demo is based on code from here https://learnopengl.com/#!PBR/Lighting
/// @brief the vertex passed in
layout (location = 0) in vec3 in_vert;
/// @brief the normal passed in
layout (location = 2) in vec3 in_normal;
/// @brief the in uv
layout (location = 1) in vec2 in_uv;

out struct
{
  vec3 position;
  vec3 normal;
  vec2 uv;
} vs_out;


void main()
{
  vs_out.position = in_vert;
  vs_out.normal = in_normal;
  vs_out.uv = in_uv;
}
