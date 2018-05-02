#version 400

// This code is based on code from here https://learnopengl.com/#!PBR/Lighting
layout (location = 0) out vec4 fragColour;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 LocalPos;
in vec3 Normal;
in float EyeVal;
in float NormZ;

uniform float eyeDisp      = -0.2;
uniform float eyeScale     = 1.55;
uniform vec3  eyeTranslate = vec3(0.21, 0.3, 0.0);
uniform float eyeRotation  = 7.0;
uniform float eyeWarp      = 1.0;
uniform float eyeExponent  = 3.0;
uniform float eyeThickness = 0.08;
uniform float eyeGap       = 0.19;
uniform float eyeFuzz      = 0.02;

// material parameters
uniform sampler3D surfaceMap;
uniform sampler3D normalMap;
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
uniform vec3 offsetPos = vec3(0.0);


// lights
const float scale = 10.0f;
const float lheight = 4.0f;
const vec3 lightPositions[4] = vec3[4](
      vec3(-scale, lheight, -scale),
      vec3( scale, lheight, -scale),
      vec3(-scale, lheight,  scale),
      vec3( scale, lheight,  scale)
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

#include "shaders/include/gpu_noise_lib.h"
#include "shaders/include/owl_noise_funcs.h"
#include "shaders/include/owl_eye_funcs.h"
#include "shaders/include/owl_bump_funcs.h"

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

void main()
{
  float normalStrength = 0.05;
  vec3 coord = LocalPos * 0.2 + vec3(0.5, 0.55, 0.5);
  // Extract the normal from the normal map (rescale to [-1,1]
  vec3 tgt = normalize((texture(normalMap, coord).rgb * 2.0 - 1.0) * normalStrength);

  // The source is just up in the Z-direction
  vec3 src = vec3(0.0, 0.0, 1.0);

  // Perturb the normal according to the target
  vec3 np = rotateVector(src, tgt, Normal);

  vec4 albedoDisp = texture(surfaceMap, coord);
  vec3 eyeAlbedo = mix(albedoDisp.xyz, vec3(0.7, 0.64, 0.68) * turb(offsetPos, 10), EyeVal*0.75);

  vec3 N = mix(np, Normal, EyeVal);
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
    Lo += (kD * eyeAlbedo / PI + brdf) * radiance * NdotL;
  }


  vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

  vec3 kS = F;
  vec3 kD = 1.0 - kS;
  kD *= (1.0 - metallic);

  vec3 irradiance = texture(irradianceMap, N).rgb;
  vec3 diffuse    = irradiance * eyeAlbedo;


  const float MAX_REFLECTION_LOD = 4.0;
  vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
  vec2 envBRDF  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
  float specularCoef = (0.5 + Perlin3D(offsetPos * 5.0) * 0.2) * (1 - EyeVal * 2.0);
  vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y) * specularCoef;

  vec3 ambient  = (kD * diffuse + specular) * ao;

  vec3 color = ambient + Lo;

  // HDR tonemapping
  color = color / (color + vec3(1.0));
  // gamma correct
  color = pow(color, vec3(1.0/2.2));

  fragColour = vec4(color, 1.0);
}
