#version 400

layout(triangles, equal_spacing, cw) in;

in struct
{
  vec3 position;
  vec3 normal;
  vec2 uv;
} tc_out[];

out struct
{
  vec3 position;
  vec3 normal;
  vec2 uv;
} te_out;

uniform mat4 MVP;

void main(void)
{
  te_out.normal   = (gl_TessCoord.x * tc_out[0].normal   + gl_TessCoord.y * tc_out[1].normal   + gl_TessCoord.z * tc_out[2].normal);
  te_out.uv       = (gl_TessCoord.x * tc_out[0].uv       + gl_TessCoord.y * tc_out[1].uv       + gl_TessCoord.z * tc_out[2].uv);
  te_out.position = (gl_TessCoord.x * tc_out[0].position + gl_TessCoord.y * tc_out[1].position + gl_TessCoord.z * tc_out[2].position);
  gl_Position = MVP * vec4(te_out.position, 1.0);
}
