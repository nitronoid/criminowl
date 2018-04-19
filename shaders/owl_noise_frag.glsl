#version 410 core

layout(location=0) out vec4 FragColor;
in vec2 TexCoords;

const float scale = 4.0;

uniform float Zdepth;
uniform vec3 offsetPos = vec3(0.0);

#include "shaders/include/gpu_noise_lib.h"
#include "shaders/include/owl_noise_funcs.h"
// ----------------------------------------------------------------------------
vec4 calcAlbedoDisp()
{
  vec3 pos = vec3(TexCoords * scale, Zdepth * scale);
  vec3 randP = pos + offsetPos; 
  float layers[] = float[](
    // large darken
    (1 - clamp(blendNoise(randPos(randP + vec3(1.0,2.0,0.0), 4, 5), 0.005) * 0.25 + 0.25, 0.0, 1.0)),
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

  vec3 cols[] = vec3[](
    vec3(0.016, 0.004, 0.0005),
    vec3(0.03, 0.009, 0.0),
    vec3(0.08, 0.002, 0.0),
    vec3(0.703, 0.188, 0.108),
    vec3(0.707, 0.090, 0.021),
    vec3(0.960, 0.436, 0.149),
    vec3(0.843, 0.326, 0.176),
    vec3(1, 0.31, 0.171)
  );
  
  vec4 result = vec4(0.093, 0.02, 0.003, 0.0);

  for (int i = 0; i < 8; ++i)
  {
    result.xyz = mix(result.xyz, cols[i], layers[i]);
    result.w += layers[i];
  }

  return result;
}

void main() 
{
    FragColor = calcAlbedoDisp();
}
