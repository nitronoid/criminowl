#version 430

layout(vertices = 3) out;

in struct
{
  vec3 position;
  vec3 base_position;
  vec3 normal;
  vec3 base_normal;
  vec2 uv;
} vs_out[];

out struct
{
  vec3 position;
  vec3 base_position;
  vec3 normal;
  vec3 base_normal;
  vec2 uv;
  vec3 phong_patch;
  float tess_mask;
} tc_out[];

uniform int u_tessLevelInner = 1;
uniform int u_tessLevelOuter = 1;
uniform float u_eyeRotation = 7.0;
uniform float u_eyeScale     = 1.55;
uniform vec3  u_eyeTranslate = vec3(0.21, 0.3, 0.0);
uniform float u_eyeFuzz      = 0.02;
uniform float u_tessMaskCap = 1.0;

#include "shaders/include/owl_eye_funcs.h"
#define ID gl_InvocationID

float PIi(int i, vec3 q)
{
 vec3 q_minus_p = q - vs_out[i].position;
 return q[ID] - dot(q_minus_p, vs_out[i].normal) * vs_out[i].normal[ID];
}

void main(void)
{
  // compute patch data
  for (int i = 0; i < 3; ++i)
  {
    int j = (i + 1) % 3;
    tc_out[ID].phong_patch[i] = PIi(i, vs_out[j].position) + PIi(j, vs_out[i].position);
  }

  vec3 pos = vs_out[ID].base_position;
  float rotation = radians(u_eyeRotation);
  vec3 posA = eyePos(pos, u_eyeScale, u_eyeTranslate, rotation);
  pos.x *= -1.0;
  vec3 posB = eyePos(pos, u_eyeScale, u_eyeTranslate, -rotation);

  float tessMask = mask(eyeMask(posA, u_eyeFuzz, u_tessMaskCap), eyeMask(posB, u_eyeFuzz, u_tessMaskCap), u_eyeFuzz, vs_out[ID].base_normal.z);
  tc_out[ID].tess_mask = tessMask;

  tc_out[ID].position = vs_out[ID].position;
  tc_out[ID].base_position = vs_out[ID].base_position;
  tc_out[ID].normal = vs_out[ID].normal;
  tc_out[ID].base_normal = vs_out[ID].base_normal;
  tc_out[ID].uv = vs_out[ID].uv;

  gl_TessLevelInner[0] = 1 + int(ceil(u_tessLevelInner * smoothstep(0.0, 1.0, tessMask)));;
  gl_TessLevelOuter[gl_InvocationID] = 1 + int(ceil(u_tessLevelOuter * smoothstep(0.0, 1.0, tessMask)));;
}
