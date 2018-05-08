#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

//in
in struct
{
  vec3 position;
  vec3 base_position;
  vec3 normal;
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

#include "shaders/include/owl_eye_funcs.h"
#include "shaders/include/owl_bump_funcs.h"
#include "shaders/include/perlin_noise.h"
#include "shaders/include/owl_noise_funcs.h"

float height(vec3 _pos, float _z)
{
  float rotation = radians(u_eyeRotation);
  vec3 posA = eyePos(_pos, u_eyeScale, u_eyeTranslate, rotation);
  _pos.x *= -1.0;
  vec3 posB = eyePos(_pos, u_eyeScale, u_eyeTranslate, -rotation);
  float maskA = eyeMask(posA, u_eyeFuzz, 0.7);
  float maskB = eyeMask(posB, u_eyeFuzz, 0.7);
  float bigMask = mask(maskA, maskB, u_eyeFuzz, _z);
  return eyes(posA, posB, u_eyeFuzz, u_eyeGap, u_eyeThickness, u_eyeWarp, u_eyeExponent, maskA, maskB) * bigMask;
}

/**
  * Compute the first difference around a point based on the surface normal.
  * The parametric formula for a point on the plane can be given by
  * x = (u, v, -(nx/nz)u - (ny/nz)v - (n.p)/nz)
  *   = (u, v, au + bv + c)
  */
vec3 firstDifferenceEstimator(vec3 p, vec3 n, float _z, float delta)
{
  float a = -(n.x/n.z);
  float b = -(n.y/n.z);
  float c = (n.x*p.x+n.y*p.y+n.z*p.z)/n.z;
  float halfdelta = 0.5 * delta;
  float invdelta = 1.0 / delta;

  const vec2 size = vec2(delta, 0.0);
  const vec2 off = vec2(-1, 1) * halfdelta;

  float s00 = height(p + vec3(off.xx, a * off.x + b * off.x + c), _z);
  float s01 = height(p + vec3(off.xy, a * off.x + b * off.y + c), _z);
  float s10 = height(p + vec3(off.yx, a * off.y + b * off.x + c), _z);
  float s11 = height(p + vec3(off.yy, a * off.y + b * off.y + c), _z);


  return vec3( 0.5*((s10-s00)+(s11-s01))*invdelta,
               0.5*((s01-s00)+(s11-s10))*invdelta,
               1.0);;
}

void main()
{
  vec3 norms[3];
  vec3 newPositions[3];
  float hVals[3];
  for(int i = 0; i < 3; i++)
  {
    norms[i] = normalize(te_out[i].normal);
    hVals[i] = height(te_out[i].base_position, te_out[i].normal.z);
    newPositions[i] = te_out[i].position + normalize(te_out[i].normal) * hVals[i] * u_eyeDisp;
  }

  vec3 faceNormal = cross(newPositions[1] - newPositions[0], newPositions[2] - newPositions[0]);

  for(int i = 0; i < 3; i++)
  {
    go_out.base_position = te_out[i].base_position + norms[i] * hVals[i] * u_eyeDisp;
    go_out.eyeVal = hVals[i];
    go_out.uv = te_out[i].uv;

//    vec3 fd = normalize(vec3(u_eyeDisp, u_eyeDisp, 1.0) * firstDifferenceEstimator(go_out.base_position, go_out.normal, te_out[i].normal.z, 0.1));

//    // Calls our normal perturbation function
//    vec3 n1 = perturbNormalVector(go_out.normal, fd);
    vec3 randNorm = normalize(faceNormal + noiseFunction(faceNormal * i) * 0.01);
    go_out.normal = mix(norms[i], randNorm, hVals[i]);
    vec4 newPos = vec4(newPositions[i], 1.0);
    go_out.world_position = vec3(M * newPos);

    gl_Position = MVP * newPos;

    EmitVertex();
  }
  EndPrimitive();
}
