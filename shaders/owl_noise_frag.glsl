#version 430 core

layout(location=0) out vec4 FragColor;
in vec2 vs_texCoords;

const float k_scale = 5.0;

uniform float u_zDepth;
uniform vec3 u_offsetPos = vec3(1.0);
uniform vec4 u_cols[9];

#include "shaders/include/perlin_noise.h"
#include "shaders/include/owl_noise_funcs.h"
// ----------------------------------------------------------------------------
vec4 calcAlbedoDisp()
{
  vec3 pos = vec3(vs_texCoords * k_scale, u_zDepth * k_scale);
  vec3 randP = (pos + u_offsetPos);
  float layers[] = float[](
    // large darken
    1 - clamp(1.0,0.0,blendNoise(randPos(randP + vec3(1,0,0), 4, 5), 0.005)),
    // thin darkening noise
    turb(randP, 4) * blendNoise(randPos(pos, 2, 15), 0.01) * 0.5,
    // small variance
    turb(randP, 4) * blendNoise(randP, 2) * 2,
    // light brushed
    brushed(randP, 0.25, vec3(20.0,1.0,1.0)) * slicednoise(randPos(randP, 2), 0.5, 5, 0.2),
    // dark brushed
    brushed(randP, 0.5, vec3(5.0,25.0,1.0)) * slicednoise(randPos(randP, 3), 0.6, 3, 0.5),
    // rough wood
    veins(randP, 6, 10) * slicednoise(randPos(randP, 1), 0.3, 3, 1.5),
    // veins
    veins(randP, 4, 2) * slicednoise(randPos(randP, 4), 1, 1.25, 0.15) * 2,
    // wood chips
    slicednoise(randP, 2.0, 0.04, 0.4)
  );
  
  vec4 result = u_cols[0];

  for (int i = 0; i < 8; ++i)
  {
    result.xyz = mix(result.xyz, u_cols[i + 1].xyz, layers[i]);
    result.w += layers[i];
  }

  return result;
}

void main() 
{
    FragColor = calcAlbedoDisp();
}
