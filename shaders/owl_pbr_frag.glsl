#version 420 // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable
// This code is based on code from here https://learnopengl.com/#!PBR/Lighting
layout (location = 0) out vec4 fragColour;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 ObjectPos;
in vec3 ObjectNormal;
in vec3 Normal;

// material parameters
uniform vec3  albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;
// camera parameters
uniform vec3 camPos;
uniform float exposure;


uniform float eyeScale     = 1.55;
uniform vec3  eyeTranslate = vec3(0.21, 0.3, 0.0);
uniform float eyeRotation  = 7.0;
uniform float eyeWarp      = 1.0;
uniform float eyeExponent  = 3.0;
uniform float eyeThickness = 0.04;
uniform float eyeGap       = 0.19;
uniform float eyeFuzz      = 0.02;

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

mat4 rotationMatrix4d(vec3 axis, float angle)
{
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;

  return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
              oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
              oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
              0.0,                                0.0,                                0.0,                                1.0);
}

mat3 rotationMatrix3d(vec3 axis, float angle)
{
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;

  return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
              oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
              oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

// Linear interpolation at between x and y, at t
float lerp(float x, float y, float t)
{
  return (1 - t) * x + t * y;
}

// adapted from larry gritz advanced renderman patterns.h
// Combines two smooth steps to create a smooth bump, 0 -> 1 -> 0
float smoothpulse (float e0, float e1, float e2, float e3, float x)
{
    return smoothstep(e0,e1,x) - smoothstep(e2,e3,x);
}

// Creates an infinite trail of smooth bumps
float smoothpulsetrain (float e0, float e1, float e2, float e3, float period, float x)
{
    return smoothpulse(e0, e1, e2, e3, mod(x,period));
}

// Wrapper for smoothpulsetrain that assumes the smoothpulse is uniform
float smoothpulsetraineven (float e0, float e1, float fuzz, float period, float x)
{
    return smoothpulsetrain(e0-fuzz, e0, e1, e1+fuzz, period, x);
}

float eyezone (vec3 pos, float fuzz, float gap, float thickness, float warp, float expo)
{
  float recipExpo = 1.0 / expo;
  // calculate the current radius
  float r = sqrt(pos.x * pos.x + pos.y * pos.y);
  // calculate the sum of the normalised x and y
  float sum = (pos.x + pos.y) / r;
  // calculate the period of the pulse train based on our sum and exp
  float period = gap * lerp(1, pow(sum,recipExpo), warp);
  float elipses = 1 - smoothpulsetraineven(thickness, thickness + period, fuzz, period, pow(r, recipExpo));

  float x = pos.x-(0.7);
  float y = pos.y-(0.7);
  float h = sqrt(x*x + y*y);
  float mask = 1.0 - smoothstep(0.95, 1.0, h);
  return  mask * clamp(elipses, 0.0, 1.0);
}

float eyes(vec3 _pos, vec3 norm, float scale, vec3 translate, float twist, float fuzz, float gap, float thickness, float warp, float expo)
{
  vec3 posA = _pos;
  vec3 posB = _pos;
  // Add some noise based on P, and scale in x
//  posA += noise("perlin", posA) * 0.066666;
//  posB += noise("perlin", posA) * 0.066666;
  float stretch = 1.15;
  posA.y *= stretch;
  posB.x *= -1.0;
  posB.y *= stretch;

  // Position the eye
  posA = posA/scale - vec3(translate);
  posB = posB/scale - vec3(translate);

  vec3 upVec = vec3(0.0, 0.0, 1.0);
  posA = rotationMatrix3d(upVec, twist) * posA;
  posB = rotationMatrix3d(upVec, -twist) * posB;

  float eyes = eyezone(posA, fuzz, gap, thickness /*+ noise("perlin", posA/2)/20*/, warp, expo)
             + eyezone(posB, fuzz, gap, thickness /*+ noise("perlin", posB/2)/20*/, warp, expo);
  if(norm.z < 0.0) eyes = 0.0;

  return eyes;
}

void main()
{

  vec3 N = normalize(Normal);

  float eyeVal = eyes(ObjectPos, ObjectNormal, eyeScale, eyeTranslate, radians(eyeRotation), eyeFuzz, eyeGap, eyeThickness, eyeWarp, eyeExponent);
  vec3 eyeAlbedo = vec3(eyeVal);

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
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

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

  // ambient lighting (note that the next IBL tutorial will replace
  // this ambient lighting with environment lighting).
  vec3 ambient = vec3(0.03) * eyeAlbedo * ao;

  vec3 color = ambient + Lo;

  // HDR tonemapping
  color = color / (color + vec3(1.0));
  // gamma correct
  color = pow(color, vec3(1.0/2.2));

  fragColour = vec4(color, 1.0);
}
