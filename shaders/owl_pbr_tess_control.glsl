#version 400

layout(vertices = 3) out;

in vec3 v_position[];
in vec3 v_normal[];
in vec2 v_uv[];

out vec3 tc_position[];
out vec3 tc_normal[];
out vec2 tc_uv[];

uniform float eyeRotation = 7.0;
uniform float eyeScale     = 1.55;
uniform vec3  eyeTranslate = vec3(0.21, 0.3, 0.0);
uniform float eyeFuzz      = 0.02;

#include "shaders/include/owl_eye_funcs.h"
#define ID gl_InvocationID


void main(void)
{
  vec3 pos = v_position[ID];
  float rotation = radians(eyeRotation);
  vec3 posA = eyePos(pos, eyeScale, eyeTranslate, rotation);
  pos.x *= -1.0;
  vec3 posB = eyePos(pos, eyeScale, eyeTranslate, -rotation);


  float maskA = eyeMask(posA, eyeFuzz, 0.7);
  float maskB = eyeMask(posB, eyeFuzz, 0.7);
  float bigMask = mask(maskA, maskB, eyeFuzz, v_normal[ID].z);

  tc_position[ID] = v_position[ID];
  tc_normal[ID] = v_normal[ID];
  tc_uv[ID] = v_uv[ID];

  float tessMask = mask(eyeMask(posA, eyeFuzz, 1.0), eyeMask(posB, eyeFuzz, 1.0), eyeFuzz, v_normal[ID].z);
  int tessLevel = 1 + int(ceil(64 * smoothstep(0.0, 1.0, tessMask)));

  gl_TessLevelInner[0] = tessLevel;
  gl_TessLevelOuter[0] = tessLevel;
  gl_TessLevelOuter[1] = tessLevel;
  gl_TessLevelOuter[2] = tessLevel;

}
