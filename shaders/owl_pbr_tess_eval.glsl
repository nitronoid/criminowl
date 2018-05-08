#version 430

#extension GL_EXT_tessellation_shader : enable

layout(triangles, equal_spacing, cw) in;

in struct
{
  vec3 position;
  vec3 base_position;
  vec3 normal;
  vec3 base_normal;
  vec2 uv;
  vec3 phong_patch;
  float tess_mask;
} tc_out[];

out struct
{
  vec3 position;
  vec3 base_position;
  vec3 normal;
  vec3 base_normal;
  vec2 uv;
} te_out;

uniform mat4 MVP;
uniform float u_phong_strength = 0.55;

#define coord gl_TessCoord

// The signature for our position function
subroutine vec3 tessFuncType(vec3);

// This uniform variable indicates which fractal function to use
subroutine uniform tessFuncType u_tessFunction;

subroutine(tessFuncType) vec3 phongTess(vec3 baryPos)
{
  vec3 coord2 = coord * coord;
  vec3 terms[3] = vec3[3](vec3(0.0),vec3(0.0),vec3(0.0));
  for (int i = 0; i < 3; ++i)
  {
    terms[i] = vec3(tc_out[0].phong_patch[i],tc_out[1].phong_patch[i],tc_out[2].phong_patch[i]);
  }
  vec3 phongPos = vec3(0.0);
  for (int i = 0; i < 3; ++i)
  {
    int j = (i + 1) % 3;
    phongPos += (coord2[i] * tc_out[i].position + coord[i] * coord[j] * terms[i]);
  }
  return mix(baryPos, phongPos, u_phong_strength);
}

subroutine(tessFuncType) vec3 flatTess(vec3 baryPos)
{
  return baryPos;
}

void main(void)
{

  vec3 baryPos = vec3(0.0);
  for (int i = 0; i < 3; ++i)
  {
    baryPos += coord[i] * tc_out[i].position;
  }

  te_out.normal   = (coord.x * tc_out[0].normal   + coord.y * tc_out[1].normal   + coord.z * tc_out[2].normal);
  te_out.base_normal   = (coord.x * tc_out[0].base_normal   + coord.y * tc_out[1].base_normal   + coord.z * tc_out[2].base_normal);
  te_out.uv       = (coord.x * tc_out[0].uv       + coord.y * tc_out[1].uv       + coord.z * tc_out[2].uv);
  te_out.position = u_tessFunction(baryPos);
  te_out.base_position = (gl_TessCoord.x * tc_out[0].base_position + gl_TessCoord.y * tc_out[1].base_position + gl_TessCoord.z * tc_out[2].base_position);
  gl_Position = MVP * vec4(te_out.position, 1.0);
}
