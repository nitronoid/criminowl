
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