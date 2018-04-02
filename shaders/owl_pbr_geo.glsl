#version 400

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

//in
in vec3  te_position[3];
in vec3  te_normal[3];
in vec2  te_uv[3];
in vec3  te_posA[3];
in vec3  te_posB[3];
in float te_maskA[3];
in float te_maskB[3];
in float te_bigMask[3];

//out
out vec2 TexCoords;
out vec3 WorldPos;
out vec3 LocalPos;
out vec3 Normal;
out float EyeVal;
out float NormZ;

uniform mat4 M;
uniform mat4 MVP;
uniform vec3 camPos;

uniform float eyeDisp      = -0.2;
uniform float eyeScale     = 1.55;
uniform vec3  eyeTranslate = vec3(0.21, 0.3, 0.0);
uniform float eyeRotation  = 7.0;
uniform float eyeWarp      = 1.0;
uniform float eyeExponent  = 3.0;
uniform float eyeThickness = 0.08;
uniform float eyeGap       = 0.19;
uniform float eyeFuzz      = 0.02;

#include "shaders/include/owl_eye_funcs.h"
#include "shaders/include/owl_bump_funcs.h"

float height(vec3 _pos, float _z)
{
  float rotation = radians(eyeRotation);
  vec3 posA = eyePos(_pos, eyeScale, eyeTranslate, rotation);
  _pos.x *= -1.0;
  vec3 posB = eyePos(_pos, eyeScale, eyeTranslate, -rotation);
  float maskA = eyeMask(posA, eyeFuzz, 0.7);
  float maskB = eyeMask(posB, eyeFuzz, 0.7);
  float bigMask = mask(maskA, maskB, eyeFuzz, _z);
  return eyes(posA, posB, eyeFuzz, eyeGap, eyeThickness, eyeWarp, eyeExponent, maskA, maskB) * bigMask;
}

void main()
{
  for(int i = 0; i < 3; i++)
  {
    NormZ = te_normal[i].z;
    Normal = normalize(te_normal[i]);
    TexCoords = te_uv[i];

    vec3 offset = vec3(1.0, 1.0, 0.0);
    float h = height(te_position[i], te_normal[i].z);

    EyeVal = h;

    vec4 newPos = vec4(te_position[i] + Normal * h * eyeDisp, 1.0);

    LocalPos = newPos.xyz;
    
    WorldPos = vec3(M * newPos);

    gl_Position = MVP * newPos;

    EmitVertex();
  }
  EndPrimitive();
}
