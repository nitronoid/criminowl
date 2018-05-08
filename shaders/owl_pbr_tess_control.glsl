#version 430

layout(vertices = 3) out;

in struct
{
  vec3 position;
  vec3 base_position;
  vec3 normal;
  vec2 uv;
} vs_out[];

out struct
{
  vec3 position;
  vec3 base_position;
  vec3 normal;
  vec2 uv;
  vec3 phong_patch;
  float tess_mask;
} tc_out[];


uniform float u_eyeRotation = 7.0;
uniform float u_eyeScale     = 1.55;
uniform vec3  u_eyeTranslate = vec3(0.21, 0.3, 0.0);
uniform float u_eyeFuzz      = 0.02;

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

  float maskA = eyeMask(posA, u_eyeFuzz, 0.7);
  float maskB = eyeMask(posB, u_eyeFuzz, 0.7);
  float bigMask = mask(maskA, maskB, u_eyeFuzz, vs_out[ID].normal.z);

  tc_out[ID].position = vs_out[ID].position;
  tc_out[ID].base_position = vs_out[ID].base_position;
  tc_out[ID].normal = vs_out[ID].normal;
  tc_out[ID].uv = vs_out[ID].uv;

  float tessMask = mask(eyeMask(posA, u_eyeFuzz, 1.0), eyeMask(posB, u_eyeFuzz, 1.0), u_eyeFuzz, vs_out[ID].normal.z);
  int tessLevel = 1 + int(ceil(63 * smoothstep(0.0, 1.0, tessMask)));
  tc_out[ID].tess_mask = tessMask;

  gl_TessLevelInner[0] = tessLevel;
  gl_TessLevelOuter[gl_InvocationID] = tessLevel;
}
