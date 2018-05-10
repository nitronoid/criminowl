#version 410 core
out vec4 fragColor;
in vec3 vs_localPos;

uniform samplerCube u_envMap;
uniform float u_roughness;


#include "shaders/include/pbr_funcs.h"

const float k_PI = 3.14159265359;

// ----------------------------------------------------------------------------

void main()
{
  // assume that v and r are equal to the normal to simplify
  vec3 n, r, v;
  n = r = v = normalize(vs_localPos);

  const uint k_sample_count = 1024u;
  vec3 prefilteredColor = vec3(0.0);
  float totalWeight = 0.0;

  for(uint i = 0u; i < k_sample_count; ++i)
  {
    vec3 h, l;
    generateImportanceSample(i, k_sample_count, u_roughness, n, v, h, l);

    float n_dot_l = max(dot(n, l), 0.0);
    if(n_dot_l > 0.0)
    {
      // sample from the environment's mip level based on roughness/pdf
      float d   = trowbridgeReitzGGX(n, h, u_roughness);
      float n_dot_h = max(dot(n, h), 0.0);
      float h_dot_v = max(dot(h, v), 0.0);
      float pdf = d * n_dot_h / (4.0 * h_dot_v) + 0.0001;

      float resolution = 512.0; // resolution of source cubemap (per face)
      float saTexel  = 4.0 * k_PI / (6.0 * resolution * resolution);
      float saSample = 1.0 / (float(k_sample_count) * pdf + 0.0001);

      float mipLevel = u_roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

      prefilteredColor += textureLod(u_envMap, l, mipLevel).rgb * n_dot_l;
      totalWeight      += n_dot_l;
    }
  }

  prefilteredColor = prefilteredColor / totalWeight;

  fragColor = vec4(prefilteredColor, 1.0);
}
