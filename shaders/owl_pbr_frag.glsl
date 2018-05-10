#version 450 core
#extension GL_EXT_sparse_texture2 : enable

// This code is based on code from here https://learnopengl.com/#!PBR/Lighting
layout (location = 0) out vec4 FragColour;

in struct
{
  vec3 world_position;
  vec3 base_position;
  vec3 normal;
  vec2 uv;
  float eyeVal;
} go_out;

// material parameters
uniform sampler3D u_albedoMap;
uniform sampler3D u_normalMap;
uniform float u_metallic = 0.0;
uniform float u_roughness = 0.0;
uniform float u_ao = 0.0;
uniform float u_baseSpec = 0.0;
uniform float u_normalStrength = 0.0;
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

const float k_lightIntensity = 100;
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
#include "shaders/include/pbr_funcs.h"


void main()
{
  // We use the base position to look-up our textures so that animation doesn't slide through
  vec3 coord = go_out.base_position * 0.2 + vec3(0.5, 0.55, 0.5);

  // Retrieve our normal map value
  vec4 normalAdjust = texture(u_normalMap, coord);

  // Extract the normal from the normal map (rescale to [-1,1]
  vec3 tgt = normalAdjust.rgb * 2.0 - 1.0;

  // The source is just up in the Z-direction
  vec3 src = vec3(0.0, 0.0, 1.0);

  // Perturb the normal according to the target
  vec3 perturbedNormal = normalize(mix(go_out.normal, rotateVector(src, tgt, go_out.normal), u_normalStrength));

  // Get the albedo map val
  vec4 albedoDisp = texture(u_albedoMap, coord);
  // Apply new albedo for the eyes
  vec3 eyeAlbedo = mix(albedoDisp.xyz, vec3(0.4, 0.34, 0.38) * turb(u_offsetPos, 10), go_out.eyeVal);

  // Final inputs to the reflectance equation
  vec3 n = perturbedNormal;
  vec3 v = normalize(u_camPos - go_out.world_position);


  // use albedo as f0 when metallic, otherwise use the set uniform
  vec3 f0 = mix(vec3(u_baseSpec), eyeAlbedo, u_metallic);

  // reflectance equation
  vec3 light_out = vec3(0.0);
  for(int i = 0; i < 4; ++i)
  {
    vec3 trans = vec3(0.0, 0.0, -2.0);
    vec3 ray = k_lightPositions[i] - go_out.world_position + trans;
    // calculate per-light radiance
    vec3 l = normalize(ray);
    vec3 h = normalize(v + l);
    float dist = length(ray);
    float attenuation = 1.0 / (dist * dist);
    vec3 radiance = k_lightColors[i] * attenuation;

    // Cook-Torrance BRDF
    float ndf = trowbridgeReitzGGX(n, h, u_roughness);
    float g   = geometrySmith(n, v, l, u_roughness);
    vec3  f   = fresnelSchlick(max(dot(h, v), 0.0), f0);

    vec3 nominator    = ndf * g * f;
    float denominator = 4 * max(dot(v, n), 0.0) * max(dot(l, n), 0.0) + 0.0001; // 0.0001 to prevent divide by zero.
    vec3 brdf = nominator / denominator;

    // diffuse = 1 - spec, where spec = fresnel, this means that energy is conserved.
    // pure metals have no diffuse lighting so we must remove it here
    vec3 diffuse = diffuseTerm(f, u_metallic);

    // scale light by NdotL
    float n_dot_l = max(dot(n, l), 0.0);

    // add to outgoing radiance light_out
    light_out += (diffuse * eyeAlbedo / k_PI + brdf) * radiance * n_dot_l;
  }


  vec3 f = fresnelSchlickRoughness(max(dot(n, v), 0.0), f0, u_roughness);

  vec3 irradiance = texture(u_irradianceMap, n).rgb;
  vec3 diffuse = diffuseTerm(f, u_metallic) * (irradiance * eyeAlbedo);


  const float MAX_REFLECTION_LOD = 4.0;
  vec3 r = reflect(-v, n);
  vec3 prefilteredColor = textureLod(u_prefilterMap, r,  u_roughness * MAX_REFLECTION_LOD).rgb;
  vec2 envBRDF  = texture(u_brdfMap, vec2(max(dot(n, v), 0.0), u_roughness)).rg;
  vec3 specular = prefilteredColor * (f * envBRDF.x + envBRDF.y);

  float ao = clamp(albedoDisp.w, 0.0, 1.0);
  vec3 ambient  = (diffuse + specular);
  ambient = mix(ambient, ambient * ao, u_ao);

  vec3 color = ambient + light_out;

  // HDR tonemaping
  color = color / (color + vec3(1.0));
  // gamma correct
  color = pow(color, vec3(1.0/2.2));

  FragColour = vec4(color, 1.0);
}
