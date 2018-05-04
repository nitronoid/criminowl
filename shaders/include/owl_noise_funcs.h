
float noiseFunction(vec3 pos)
{
  return SimplexPerlin3D(pos);
}

float turb (vec3 _pos, float _frequency)
{
  vec3 pos = _pos;
  float ret = 0;
  float frequency = _frequency;
  for(int i = 0; i < 8; ++i)
  {
    ret += abs(noiseFunction(pos*frequency))/frequency;
    frequency*=2.1;
  }
  return ret;
}

float slicednoise (vec3 pos, float frequency, float fuzz, float slice)
{
  return smoothstep(slice, slice + fuzz, turb(pos, frequency));
}

float brushed (vec3 _pos, float frequency, vec3 stretch)
{
  vec3 pos = _pos;
  pos += noiseFunction(pos*frequency)/frequency;
  pos *= stretch;
  return turb(pos, frequency*2);
}

float dots(vec3 _pos)
{
  vec3 pos = _pos;
  return 1 - slicednoise(pos, 4, 0.01, 0.02);
}

float veins(vec3 _pos, float frequency, float stretch)
{
  vec3 pos = _pos;
  pos[0] *= stretch;
  return 1 - slicednoise(pos, frequency, 0.05, 0.01);
}

vec3 randPos(vec3 _pos, float rand, float scale)
{
  return _pos + noiseFunction(_pos + rand) * scale;
}

vec3 randPos(vec3 _pos, float rand)
{
  return _pos + noiseFunction(_pos + rand);
}

float blendNoise(vec3 pos, float freq)
{
  return noiseFunction(randPos(pos,0,1) * freq)/freq;
}
