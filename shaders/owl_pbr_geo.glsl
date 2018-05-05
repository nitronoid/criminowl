#version 400

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

//in
in struct
{
  vec3 position;
  vec3 normal;
  vec2 uv;
} te_out[3];

//out
out struct
{
  vec3 localPos;
  vec3 worldPos;
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

  float u = -halfdelta;
  float v = -halfdelta;
  //float c00 = texture(tex, p + vec3(u,v,a*u+b*v+c)).r;
  float c00 = height(p + vec3(u,v,0), _z);

  u = -halfdelta; v = halfdelta;
  //float c01 = texture(tex, p + vec3(u,v,a*u+b*v+c)).r;
  float c01 = height(p + vec3(u,v,0), _z);

  u = halfdelta; v = -halfdelta;
  //float c10 = texture(tex, p + vec3(u,v,a*u+b*v+c)).r;
  float c10 = height(p + vec3(u,v,0), _z);

  u = halfdelta; v = halfdelta;
  //float c11 = texture(tex, p + vec3(u,v,a*u+b*v+c)).r;
  float c11 = height(p + vec3(u,v,0), _z);

  return vec3( 0.5*((c10-c00)+(c11-c01))*invdelta,
               0.5*((c01-c00)+(c11-c10))*invdelta,
               1.0);
}

void main()
{
  for(int i = 0; i < 3; i++)
  {
    go_out.normal = normalize(te_out[i].normal);
    go_out.uv = te_out[i].uv;

    vec3 offset = vec3(1.0, 1.0, 0.0);
    float h = height(te_out[i].position, te_out[i].normal.z);

    go_out.eyeVal = h;

    vec4 newPos = vec4(te_out[i].position + go_out.normal * h * u_eyeDisp, 1.0);
    go_out.localPos = newPos.xyz;
    // Now calculate the specular component
    vec3 fd = normalize(vec3(u_eyeDisp, u_eyeDisp, 1.0) * firstDifferenceEstimator(go_out.localPos, go_out.normal, te_out[i].normal.z, 0.1));

    // Calls our normal perturbation function
    vec3 n1 = perturbNormalVector(go_out.normal, fd);

    go_out.normal = mix(go_out.normal, n1, h);
    go_out.worldPos = vec3(M * newPos);

    gl_Position = MVP * newPos;

    EmitVertex();
  }
  EndPrimitive();
}
