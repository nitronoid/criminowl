#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

//in
in struct
{
  vec3 position;
  vec3 base_position;
  vec3 normal;
  vec3 base_normal;
  vec2 uv;
} te_out[3];

//out
out struct
{
  vec3 world_position;
  vec3 base_position;
  vec3 normal;
  vec2 uv;
  float eyeVal;
} go_out;

uniform mat4 MVP;
uniform mat4 M;
uniform vec3 u_camPos;

uniform float u_eyeDisp      = -0.2;
uniform float u_eyeScale     = 1.55;
uniform vec3  u_eyeTranslate = vec3(0.21, 0.3, 0.0);
uniform float u_eyeRotation  = 7.0;
uniform float u_eyeWarp      = 1.0;
uniform float u_eyeExponent  = 3.0;
uniform float u_eyeThickness = 0.08;
uniform float u_eyeGap       = 0.19;
uniform float u_eyeFuzz      = 0.02;
uniform float u_eyeMaskCap   = 0.7;

#include "shaders/include/owl_eye_funcs.h"
#include "shaders/include/owl_bump_funcs.h"

float height(vec3 _pos, float _z)
{
  float rotation = radians(u_eyeRotation);
  vec3 posA = eyePos(_pos, u_eyeScale, u_eyeTranslate, rotation);
  _pos.x *= -1.0;
  vec3 posB = eyePos(_pos, u_eyeScale, u_eyeTranslate, -rotation);
  float maskA = eyeMask(posA, u_eyeFuzz, u_eyeMaskCap);
  float maskB = eyeMask(posB, u_eyeFuzz, u_eyeMaskCap);
  float bigMask = mask(maskA, maskB, u_eyeFuzz, _z);
  return eyes(posA, posB, u_eyeFuzz, u_eyeGap, u_eyeThickness, u_eyeWarp, u_eyeExponent, maskA, maskB) * bigMask;
}

void main()
{
  vec3 norms[3];
  vec3 newPositions[3];
  float hVals[3];
  for(int i = 0; i < 3; i++)
  {
    norms[i] = normalize(te_out[i].normal);
    hVals[i] = height(te_out[i].base_position, te_out[i].base_normal.z);
    newPositions[i] = te_out[i].position + normalize(te_out[i].normal) * hVals[i] * u_eyeDisp;
  }

  vec3 faceNormal = normalize(cross(newPositions[1] - newPositions[0], newPositions[2] - newPositions[0]));

  for(int i = 0; i < 3; i++)
  {
    go_out.base_position = te_out[i].base_position + te_out[i].base_normal * hVals[i] * u_eyeDisp;
    go_out.eyeVal = hVals[i];
    go_out.uv = te_out[i].uv;

    go_out.normal = mix(norms[i], faceNormal, hVals[i]);
    vec4 newPos = vec4(newPositions[i], 1.0);
    go_out.world_position = vec3(M * newPos);

    gl_Position = MVP * newPos;

    EmitVertex();
  }
  EndPrimitive();
}
