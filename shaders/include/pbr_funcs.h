#define __PBR__PI__ 3.14159265359
// ----------------------------------------------------------------------------
float trowbridgeReitzGGX(vec3 n, vec3 h, float roughness)
{
  float a = roughness*roughness;
  float a_sqr = a*a;
  float n_dot_h = max(dot(n, h), 0.0);
  float n_dot_h_sqr = n_dot_h * n_dot_h;

  float denom = (n_dot_h_sqr * (a_sqr - 1.0) + 1.0);
  denom = __PBR__PI__ * denom * denom;

  return a_sqr / denom;
}
// ----------------------------------------------------------------------------
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.
float radicalInverse_VdC(uint bits) 
{
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 hammersley(uint i, uint n)
{
  return vec2(float(i)/float(n), radicalInverse_VdC(i));
}
// ----------------------------------------------------------------------------
vec3 importanceSampleGGX(vec2 xi, vec3 n, float roughness)
{
  float a = roughness*roughness;

  float phi = 2.0 * __PBR__PI__ * xi.x;
  float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a*a - 1.0) * xi.y));
  float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

  // convert from spherical coordinates to cartesian coordinates
  // this gives us our half vector
  vec3 h = vec3(
      cos(phi) * sinTheta,
      sin(phi) * sinTheta,
      cosTheta
    );

  // convert the half vector from tangent to world space
  // this gives us our sample vector
  vec3 up        = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  vec3 tangent   = normalize(cross(up, n));
  vec3 bitangent = cross(n, tangent);

  return normalize(tangent * h.x + bitangent * h.y + n * h.z);
}
// ----------------------------------------------------------------------------
void generateImportanceSample(uint i, uint sample_count, float roughness, vec3 n, vec3 v, out vec3 h, out vec3 l)
{
    // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
  vec2 xi = hammersley(i, sample_count);
  h = importanceSampleGGX(xi, n, roughness);
  l = normalize(2.0 * dot(v, h) * h - v);
}
// ----------------------------------------------------------------------------
float schlickGGX(float n_dot_v, float roughness)
{
  float r = (roughness + 1.0);
  // Divide by 8, 1/8 = 0.125
  float k = (r*r) * 0.125;
  return n_dot_v / (n_dot_v * (1.0 - k) + k);
}
// ----------------------------------------------------------------------------
float geometrySmith(vec3 n, vec3 v, vec3 l, float roughness)
{
  float n_dot_v = max(dot(n, v), 0.0);
  float n_dot_l = max(dot(n, l), 0.0);
  float ggx2 = schlickGGX(n_dot_v, roughness);
  float ggx1 = schlickGGX(n_dot_l, roughness);

  return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickCommon(float cosTheta, vec3 f0, vec3 base)
{
  return f0 + (base - f0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 f0)
{
  return fresnelSchlickCommon(cosTheta, f0, vec3(1.0));
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
  return fresnelSchlickCommon(cosTheta, f0, max(vec3(1.0 - roughness), f0));
}
// ----------------------------------------------------------------------------
vec3 diffuseTerm(vec3 f, float metallic)
{
    return (1.0 - f) * (1.0 - metallic);
}
