#version 400

// This code is based on code from here https://learnopengl.com/#!PBR/Lighting
layout (location = 0) out vec4 fragColour;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 LocalPos;
in vec3 Normal;
in float EyeVal;

// material parameters
uniform vec3  albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;
// camera parameters
uniform vec3 camPos;
//env map params
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D   brdfLUT;
//vert params
uniform vec3 offsetPos;

// lights
const float scale = 10.0f;
const float height = 4.0f;
const vec3 lightPositions[4] = vec3[4](
      vec3(-scale, height, -scale),
      vec3( scale, height, -scale),
      vec3(-scale, height,  scale),
      vec3( scale, height,  scale)
      );

const float intensity = 100.f;
const vec3 lightColors[4] = vec3[4](
      vec3(intensity, intensity, intensity),
      vec3(intensity, intensity, intensity),
      vec3(intensity, intensity, intensity),
      vec3(intensity, intensity, intensity)
      );

// Define pi
const float PI = 3.14159265359;

#include "shaders/include/noise_funcs.h"

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
  float a = roughness*roughness;
  float a2 = a*a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH*NdotH;

  float nom   = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
  float r = (roughness + 1.0);
  float k = (r*r) / 8.0;

  float nom   = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec4 calcAlbedoDisp()
{
  vec3 randP = LocalPos + offsetPos; 
  float layers[] = float[](
    // large darken
    (1 - clamp(blendNoise(randPos(randP + vec3(1.0,2.0,0.0), 4, 5), 0.005) * 0.25 + 0.25, 0.0, 1.0)),
    // thin darkening noise
    turb(randP, 4) * blendNoise(randPos(LocalPos, 2, 15), 0.01) * 0.5,
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

  vec3 N = normalize(Normal);

  vec4 albedoDisp = calcAlbedoDisp();
  vec3 eyeAlbedo = vec3(albedoDisp.xyz);

  vec3 V = normalize(camPos - WorldPos);
  vec3 R = reflect(-V, N);


  // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
  // of 0.04 and if it's a metal, use their albedo color as F0 (metallic workflow)
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, eyeAlbedo, metallic);

  // reflectance equation
  vec3 Lo = vec3(0.0);
  for(int i = 0; i < 4; ++i)
  {
    vec3 trans = vec3(0.0, 0.0, -2.0);
    vec3 ray = lightPositions[i] - WorldPos + trans;
    // calculate per-light radiance
    vec3 L = normalize(ray);
    vec3 H = normalize(V + L);
    float dist = length(ray);
    float attenuation = 1.0 / (dist * dist);
    vec3 radiance = lightColors[i] * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 nominator    = NDF * G * F;
    float denominator = 4 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0) + 0.001; // 0.001 to prevent divide by zero.
    vec3 brdf = nominator / denominator;

    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);

    // add to outgoing radiance Lo
    Lo += (kD * eyeAlbedo / PI + brdf) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
  }


  vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);



  // ambient lighting (note that the next IBL tutorial will replace
  // this ambient lighting with environment lighting).
  vec3 kS = F;
  vec3 kD = 1.0 - kS;
  kD *= (1.0 - metallic);

  vec3 irradiance = texture(irradianceMap, N).rgb;
  vec3 diffuse    = irradiance * eyeAlbedo;


  const float MAX_REFLECTION_LOD = 4.0;
  vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
  vec2 envBRDF  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
  vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

  vec3 ambient  = (kD * diffuse + specular) * ao;

  vec3 color = ambient + Lo;

  // HDR tonemapping
  color = color / (color + vec3(1.0));
  // gamma correct
  color = pow(color, vec3(1.0/2.2));

  fragColour = vec4(color, 1.0);
}
