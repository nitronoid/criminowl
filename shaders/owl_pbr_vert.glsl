#version 430

// this demo is based on code from here https://learnopengl.com/#!PBR/Lighting
/// @brief the vertex passed in
layout (location = 0) in vec3 in_vert;
/// @brief the normal passed in
layout (location = 2) in vec3 in_normal;
/// @brief the in uv
layout (location = 1) in vec2 in_uv;

layout (std430, binding = 0) buffer morph_targets
{
  vec4 targets[];
};

out struct
{
  vec3 position;
  vec3 base_position;
  vec3 normal;
  vec2 uv;
} vs_out;

uniform int u_morph_target_size = 0;
uniform int u_morph_target_normal_offset = 0;
uniform float u_blend = 0.0;

void getTargets(float blend, out vec3 targetPos, out vec3 targetNorm)
{
  const int first = int(floor(blend));
  const int firstVertID = (u_morph_target_size * first) + gl_VertexID;
  const int firstNormID = firstVertID + u_morph_target_normal_offset;

  const int second = first + 1;
  const int secondVertID = (u_morph_target_size * second) + gl_VertexID;
  const int secondNormID = secondVertID + u_morph_target_normal_offset;

  const float inbetween = blend - first;

  targetPos = mix(targets[firstVertID], targets[secondVertID], smoothstep(0.0, 1.0, inbetween)).xyz;
  targetNorm = normalize(mix(targets[firstNormID], targets[secondNormID], smoothstep(0.0, 1.0, inbetween)).xyz);

}

void main()
{
  vec3 targetPosition, targetNormal;
  getTargets(u_blend, targetPosition, targetNormal);
  vs_out.position = targetPosition;
  vs_out.base_position = in_vert;
  vs_out.normal = targetNormal;
  vs_out.uv = in_uv;
}
