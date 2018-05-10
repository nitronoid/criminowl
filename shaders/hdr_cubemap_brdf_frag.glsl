#version 410 core

layout(location=0) out vec2 fragColor;
in vec2 vs_texCoords;

const float k_PI = 3.14159265359;
#include "shaders/include/pbr_funcs.h"

// ----------------------------------------------------------------------------
float brdfSchlickGGX(float n_dot_v, float roughness)
{
  // note that we use a different k for IBL
  float k = (roughness * roughness) * 0.5;
  return n_dot_v / (n_dot_v * (1.0 - k) + k);
}
// ----------------------------------------------------------------------------
float brdfGeometrySmith(vec3 n, vec3 v, vec3 l, float roughness)
{
  float n_dot_v = max(dot(n, v), 0.0);
  float n_dot_l = max(dot(n, l), 0.0);
  float ggx2 = brdfSchlickGGX(n_dot_v, roughness);
  float ggx1 = brdfSchlickGGX(n_dot_l, roughness);

  return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------

vec2 integrateBRDF(float n_dot_v, float roughness)
{
  vec3 v;
  v.x = sqrt(1.0 - n_dot_v*n_dot_v);
  v.y = 0.0;
  v.z = n_dot_v;

  float a = 0.0;
  float b = 0.0;

  vec3 n = vec3(0.0, 0.0, 1.0);

  const uint k_sample_count = 1024u;
  for(uint i = 0u; i < k_sample_count; ++i)
  {
    // generates a sample vector that's biased towards the
    // preferred alignment direction (importance sampling).
    vec3 h, l;
    generateImportanceSample(i, k_sample_count, roughness, n, v, h, l);

    float n_dot_l = max(l.z, 0.0);
    float n_dot_h = max(h.z, 0.0);
    float v_dot_h = max(dot(v, h), 0.0);

    if(n_dot_l > 0.0)
    {
      float g = brdfGeometrySmith(n, v, l, roughness);
      float g_vis = (g * v_dot_h) / (n_dot_h * n_dot_v);
      float fc = pow(1.0 - v_dot_h, 5.0);

      a += (1.0 - fc) * g_vis;
      b += fc * g_vis;
    }
  }
  a /= float(k_sample_count);
  b /= float(k_sample_count);
  return vec2(a, b);
}
// ----------------------------------------------------------------------------
void main() 
{
  vec2 integratedBRDF = integrateBRDF(vs_texCoords.x, vs_texCoords.y);
  fragColor = integratedBRDF;
}
