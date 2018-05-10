#version 410 core

layout(location=0) out vec4 fragColor;
in vec3 vs_localPos;

uniform samplerCube u_envMap;

const float k_PI = 3.14159265359;

void main()
{
  // Use the normal (world vector), to calculate the sum of all incoming radiance (irradiance).
  // The result is light coming from -normal, so that's what we'll use to query later.
  vec3 n = normalize(vs_localPos);

  vec3 irradiance = vec3(0.0);

  // tangent space calculation from origin point
  vec3 up    = vec3(0.0, 1.0, 0.0);
  vec3 right = cross(up, n);

  float sampleDelta = 0.025;
  float sampleCount = 0.0;
  // Iterate evenly over the hemisphere
  for(float phi = 0.0; phi < 2.0 * k_PI; phi += sampleDelta)
  {
    for(float theta = 0.0; theta < 0.5 * k_PI; theta += sampleDelta)
    {
      // convert spherical to cartesian (in tangent space)
      vec3 tgtSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
      // tangent space to world
      vec3 sampleVec = tgtSample.x * right + tgtSample.y * up + tgtSample.z * n;

      irradiance += texture(u_envMap, sampleVec).rgb * cos(theta) * sin(theta);
      ++sampleCount;
    }
  }
  irradiance *= k_PI * (1.0 / float(sampleCount));

  fragColor = vec4(irradiance, 1.0);
}
