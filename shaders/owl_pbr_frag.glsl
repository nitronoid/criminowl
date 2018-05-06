#version 400

// This code is based on code from here https://learnopengl.com/#!PBR/Lighting
layout (location = 0) out vec4 FragColour;

in struct
{
  vec3 localPos;
  vec3 worldPos;
  vec3 normal;
  vec2 uv;
  float eyeVal;
} go_out;

// material parameters
uniform sampler3D u_albedoMap;
uniform sampler3D u_normalMap;
uniform float u_metallic;
uniform float u_roughness;
uniform float u_ao;
// camera parameters
uniform vec3 u_camPos;
//env map params
uniform samplerCube u_irradianceMap;
uniform samplerCube u_prefilterMap;
uniform sampler2D   u_brdfMap;
//vert params
uniform vec3 u_offsetPos = vec3(0.0);


// lights
const float k_scale = 10.0f;
const float k_lightHeight = 4.0f;
const vec3 k_lightPositions[4] = vec3[4](
      vec3(-k_scale, k_lightHeight, -k_scale),
      vec3( k_scale, k_lightHeight, -k_scale),
      vec3(-k_scale, k_lightHeight,  k_scale),
      vec3( k_scale, k_lightHeight,  k_scale)
      );

const float k_lightIntensity = 100.f;
const vec3 k_lightColors[4] = vec3[4](
      vec3(k_lightIntensity, k_lightIntensity, k_lightIntensity),
      vec3(k_lightIntensity, k_lightIntensity, k_lightIntensity),
      vec3(k_lightIntensity, k_lightIntensity, k_lightIntensity),
      vec3(k_lightIntensity, k_lightIntensity, k_lightIntensity)
      );

// Define k_PI
const float k_PI = 3.14159265359;

#include "shaders/include/perlin_noise.h"
#include "shaders/include/owl_noise_funcs.h"
#include "shaders/include/owl_eye_funcs.h"
#include "shaders/include/owl_bump_funcs.h"

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
  float a = roughness*u_roughness;
  float a2 = a*a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH*NdotH;

  float nom   = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = k_PI * denom * denom;

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
  float normalStrength = 0.3;
  vec3 coord = go_out.localPos * 0.2 + vec3(0.5, 0.55, 0.5);
  // Extract the normal from the normal map (rescale to [-1,1]
  vec3 tgt = texture(u_normalMap, coord).rgb * 2.0 - 1.0;

  // The source is just up in the Z-direction
  vec3 src = vec3(0.0, 0.0, 1.0);

  // Perturb the normal according to the target
  vec3 np = normalize(mix(go_out.normal, rotateVector(src, tgt, go_out.normal), normalStrength));

  vec4 albedoDisp = texture(u_albedoMap, coord);
  vec3 eyeAlbedo = mix(albedoDisp.xyz, vec3(0.4, 0.34, 0.38) * turb(u_offsetPos, 10), go_out.eyeVal);

  vec3 N = np;//mix(np, Normal, EyeVal);
  vec3 V = normalize(u_camPos - go_out.worldPos);
  vec3 R = reflect(-V, N);


  // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
  // of 0.04 and if it's a metal, use their albedo color as F0 (metallic workflow)
  float baseSpec = 0.1;// + cnoise(u_offsetPos * 5.0) * 0.2 * (1 - go_out.eyeVal * 2.0);
  vec3 F0 = vec3(baseSpec);
  F0 = mix(F0, eyeAlbedo, u_metallic);

  // reflectance equation
  vec3 Lo = vec3(0.0);
  for(int i = 0; i < 4; ++i)
  {
    vec3 trans = vec3(0.0, 0.0, -2.0);
    vec3 ray = k_lightPositions[i] - go_out.worldPos + trans;
    // calculate per-light radiance
    vec3 L = normalize(ray);
    vec3 H = normalize(V + L);
    float dist = length(ray);
    float attenuation = 1.0 / (dist * dist);
    vec3 radiance = k_lightColors[i] * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, u_roughness);
    float G   = GeometrySmith(N, V, L, u_roughness);
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
    kD *= 1.0 - u_metallic;

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);

    // add to outgoing radiance Lo
    Lo += (kD * eyeAlbedo / k_PI + brdf) * radiance * NdotL;
  }


  vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, u_roughness);

  vec3 kS = F;
  vec3 kD = 1.0 - kS;
  kD *= (1.0 - u_metallic);

  vec3 irradiance = texture(u_irradianceMap, N).rgb;
  vec3 diffuse    = irradiance * eyeAlbedo;


  const float MAX_REFLECTION_LOD = 4.0;
  vec3 prefilteredColor = textureLod(u_prefilterMap, R,  u_roughness * MAX_REFLECTION_LOD).rgb;
  vec2 envBRDF  = texture(u_brdfMap, vec2(max(dot(N, V), 0.0), u_roughness)).rg;
  vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

  vec3 ambient  = (kD * diffuse + specular) * u_ao;

  vec3 color = ambient + Lo;

  // HDR tonemaping
  color = color / (color + vec3(1.0));
  // gamma correct
  color = pow(color, vec3(1.0/2.2));

  FragColour = vec4(color, 1.0);
}
