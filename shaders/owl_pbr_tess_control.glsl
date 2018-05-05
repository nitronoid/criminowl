#version 400

layout(vertices = 3) out;

in struct
{
  vec3 position;
  vec3 normal;
  vec2 uv;
} vs_out[];

out struct
{
  vec3 position;
  vec3 normal;
  vec2 uv;
} tc_out[];


uniform float u_eyeRotation = 7.0;
uniform float u_eyeScale     = 1.55;
uniform vec3  u_eyeTranslate = vec3(0.21, 0.3, 0.0);
uniform float u_eyeFuzz      = 0.02;

#include "shaders/include/owl_eye_funcs.h"
#define ID gl_InvocationID


void main(void)
{
  vec3 pos = vs_out[ID].position;
  float rotation = radians(u_eyeRotation);
  vec3 posA = eyePos(pos, u_eyeScale, u_eyeTranslate, rotation);
  pos.x *= -1.0;
  vec3 posB = eyePos(pos, u_eyeScale, u_eyeTranslate, -rotation);

  float maskA = eyeMask(posA, u_eyeFuzz, 0.7);
  float maskB = eyeMask(posB, u_eyeFuzz, 0.7);
  float bigMask = mask(maskA, maskB, u_eyeFuzz, vs_out[ID].normal.z);

  tc_out[ID].position = vs_out[ID].position;
  tc_out[ID].normal = vs_out[ID].normal;
  tc_out[ID].uv = vs_out[ID].uv;

  float tessMask = mask(eyeMask(posA, u_eyeFuzz, 1.0), eyeMask(posB, u_eyeFuzz, 1.0), u_eyeFuzz, vs_out[ID].normal.z);
  int tessLevel = 1 + int(ceil(15 * smoothstep(0.0, 1.0, tessMask)));

  gl_TessLevelInner[0] = tessLevel;
  gl_TessLevelOuter[0] = tessLevel;
  gl_TessLevelOuter[1] = tessLevel;
  gl_TessLevelOuter[2] = tessLevel;

}
