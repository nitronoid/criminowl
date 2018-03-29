#version 400

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

//in
in vec3  te_position[3];
in vec3  te_normal[3];
in vec2  te_uv[3];
in vec3  te_posA[3];
in vec3  te_posB[3];
in float te_maskA[3];
in float te_maskB[3];
in float te_bigMask[3];

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

float eyezone (vec3 pos, float fuzz, float gap, float thickness, float warp, float expo, float _mask)
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

  return clamp(elipses, 0.0, 1.0) * _mask;
}

float eyes(vec3 _posA, vec3 _posB, float fuzz, float gap, float thickness, float warp, float expo, float _maskA, float _maskB)
{
  // Add some noise based on P, and scale in x
  vec3 variance = noise3(_posA) * 0.066666;
  _posA += variance;
  _posB -= variance;


  float eyes = eyezone(_posA, fuzz, gap, thickness + noise1(_posA * 0.5) * 0.05, warp, expo, _maskA)
      + eyezone(_posB, fuzz, gap, thickness + noise1(_posB * 0.5) * 0.05, warp, expo, _maskB);


  return eyes;
}

void main( void )
{
  for(int i=0; i < 3; i++)
  {
    Normal = normalize(te_normal[i]);
    TexCoords = te_uv[i];



    float height = eyes(te_posA[i], te_posB[i], eyeFuzz, eyeGap, eyeThickness, eyeWarp, eyeExponent, te_maskA[i], te_maskB[i]) * te_bigMask[i];

    EyeVal = height;

    vec4 newPos = vec4(te_position[i] + Normal * height * eyeDisp, 1.0);
    WorldPos = vec3(M * newPos);

    gl_Position = MVP * newPos;

    EmitVertex();
  }
  EndPrimitive();
}


