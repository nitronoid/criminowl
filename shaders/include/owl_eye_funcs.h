
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

vec3 eyePos(vec3 _pos, float scale, vec3 translate, float _twist)
{
  vec3 newPos = _pos;
  float stretch = 1.15;
  newPos.y *= stretch;

  newPos = newPos/scale - vec3(translate);

  vec3 upVec = vec3(0.0, 0.0, 1.0);
  newPos = rotationMatrix3d(upVec, _twist) * newPos;

  return newPos;
}

float eyeMask(vec3 _pos, float _fuzz, float _cap)
{
  vec2 centre = vec2(0.5);
  float dist = distance(centre, _pos.xy);
  float mask = 1.0 - smoothstep(_cap - _fuzz, _cap + _fuzz, dist);
  return clamp(mask, 0.0, 1.0);
}

float mask(float _maskA, float _maskB, float _fuzz, float _zDir)
{
  return (_maskA + _maskB) * smoothstep(0.0, _fuzz, _zDir);
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
  float period = gap * mix(1, pow(sum, recipExpo), warp);
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