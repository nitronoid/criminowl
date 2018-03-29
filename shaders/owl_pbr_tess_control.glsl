#version 400

layout(vertices = 3) out;

in vec3 v_position[];
in vec3 v_normal[];
in vec2 v_uv[];

out vec3 tc_position[];
out vec3 tc_normal[];
out vec2 tc_uv[];
out vec3 tc_posA[];
out vec3 tc_posB[];
out float tc_maskA[];
out float tc_maskB[];
out float tc_bigMask[];

uniform int innerTess = 64;
uniform int outerTess = 64;
uniform float eyeRotation = 7.0;
uniform float eyeScale     = 1.55;
uniform vec3  eyeTranslate = vec3(0.21, 0.3, 0.0);
uniform float eyeFuzz      = 0.02;

#define ID gl_InvocationID

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

float eyeMask(vec3 _pos, float _fuzz)
{
  vec2 centre = vec2(0.5);
  float dist = distance(centre, _pos.xy);
  float cap = 0.7;
  float mask = 1.0 - smoothstep(cap - _fuzz, cap + _fuzz, dist);
  return clamp(mask, 0.0, 1.0);
}

float mask(float _maskA, float _maskB, float _fuzz, float _zDir)
{
  return (_maskA + _maskB) * smoothstep(0.0, 0.0 + _fuzz, _zDir);
}

void main(void)
{
  vec3 pos = v_position[ID];
  float rotation = radians(eyeRotation);
  vec3 posA = eyePos(pos, eyeScale, eyeTranslate, rotation);
  pos.x *= -1.0;
  vec3 posB = eyePos(pos, eyeScale, eyeTranslate, -rotation);

  tc_posA[ID] = posA;
  tc_posB[ID] = posB;

  float maskA = eyeMask(posA, eyeFuzz);
  float maskB = eyeMask(posB, eyeFuzz);
  tc_maskA[ID] = maskA;
  tc_maskB[ID] = maskB;
  float bigMask = mask(maskA, maskB, eyeFuzz, v_normal[ID].z);
  tc_bigMask[ID] = bigMask;

  tc_position[ID] = v_position[ID];
  tc_normal[ID] = v_normal[ID];
  tc_uv[ID] = v_uv[ID];

  int tessLevel = 1 + int(ceil(64 * smoothstep(0.0, 1.0, bigMask)));
//  if(ID == 0)
//  {
    gl_TessLevelInner[ID] = tessLevel;
    gl_TessLevelOuter[ID * 3] = tessLevel;
    gl_TessLevelOuter[ID * 3 + 1] = tessLevel;
    gl_TessLevelOuter[ID * 3 + 2] = tessLevel;
//  }
}
