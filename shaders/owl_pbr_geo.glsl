#version 400

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

//in
in vec3 te_position[3];
in vec3 te_normal[3];
in vec2 te_uv[3];

//out
out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out float EyeVal;

uniform mat4 M;
uniform mat4 MVP;
uniform vec3 camPos;

uniform float eyeDisp      = -0.2;
uniform float eyeScale     = 1.55;
uniform vec3  eyeTranslate = vec3(0.21, 0.3, 0.0);
uniform float eyeRotation  = 7.0;
uniform float eyeWarp      = 1.0;
uniform float eyeExponent  = 3.0;
uniform float eyeThickness = 0.08;
uniform float eyeGap       = 0.19;
uniform float eyeFuzz      = 0.02;


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
  float period = gap * lerp(1, pow(sum, recipExpo), warp);
  float adjustedFuzz = fuzz;
  float elipses = 1 - smoothpulsetraineven(thickness*0.5, thickness + period * 0.5, adjustedFuzz, period, pow(r, recipExpo));

  vec2 centre = vec2(0.5);
  float dist = distance(centre, pos.xy);
  float cap = 0.7;
  float mask = 1.0 - smoothstep(cap - fuzz, cap + fuzz, dist);
  return mask * clamp(elipses, 0.0, 1.0);
}

float eyes(vec3 _pos, vec3 norm, float scale, vec3 translate, float twist, float fuzz, float gap, float thickness, float warp, float expo)
{
  vec3 posA = _pos;
  vec3 posB = _pos;
  // Add some noise based on P, and scale in x
  vec3 variance = noise3(posA) * 0.066666;
  posA += variance;
  posB += variance;
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

  float eyes = eyezone(posA, fuzz, gap, thickness + noise1(posA * 0.5) * 0.05, warp, expo)
      + eyezone(posB, fuzz, gap, thickness + noise1(posB * 0.5) * 0.05, warp, expo);
  if(norm.z < 0.0) eyes = 0.0;

  return eyes;
}

void main( void )
{
  for(int i=0; i < 3; i++)
  {
    Normal = normalize(te_normal[i]);
    TexCoords = te_uv[i];

    float height = eyes(te_position[i], te_normal[i], eyeScale, eyeTranslate, radians(eyeRotation), eyeFuzz, eyeGap, eyeThickness, eyeWarp, eyeExponent);

    EyeVal = height;

    vec4 newPos = vec4(te_position[i] + Normal * height * eyeDisp, 1.0);
    WorldPos = vec3(M * newPos);

    gl_Position = MVP * newPos;

    EmitVertex();
  }
  EndPrimitive();
}


