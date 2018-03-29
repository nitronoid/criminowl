#version 400

layout(triangles, equal_spacing, cw) in;

in vec3  tc_position[];
in vec3  tc_normal[];
in vec2  tc_uv[];
in vec3  tc_posA[];
in vec3  tc_posB[];
in float tc_maskA[];
in float tc_maskB[];
in float tc_bigMask[];

out vec3  te_position;
out vec3  te_normal;
out vec2  te_uv;
out vec3  te_posA;
out vec3  te_posB;
out float te_maskA;
out float te_maskB;
out float te_bigMask;

uniform mat4 MVP;

void main(void)
{
  te_normal   = (gl_TessCoord.x * tc_normal[0]   + gl_TessCoord.y * tc_normal[1]   + gl_TessCoord.z * tc_normal[2]);
  te_uv       = (gl_TessCoord.x * tc_uv[0]       + gl_TessCoord.y * tc_uv[1]       + gl_TessCoord.z * tc_uv[2]);
  te_position = (gl_TessCoord.x * tc_position[0] + gl_TessCoord.y * tc_position[1] + gl_TessCoord.z * tc_position[2]);
  te_posA     = (gl_TessCoord.x * tc_posA[0]     + gl_TessCoord.y * tc_posA[1]     + gl_TessCoord.z * tc_posA[2]);
  te_posB     = (gl_TessCoord.x * tc_posB[0]     + gl_TessCoord.y * tc_posB[1]     + gl_TessCoord.z * tc_posB[2]);
  te_maskA    = (gl_TessCoord.x * tc_maskA[0]    + gl_TessCoord.y * tc_maskA[1]    + gl_TessCoord.z * tc_maskA[2]);
  te_maskB    = (gl_TessCoord.x * tc_maskB[0]    + gl_TessCoord.y * tc_maskB[1]    + gl_TessCoord.z * tc_maskB[2]);
  te_bigMask  = (gl_TessCoord.x * tc_bigMask[0]  + gl_TessCoord.y * tc_bigMask[1]  + gl_TessCoord.z * tc_bigMask[2]);
  gl_Position = MVP * vec4(te_position, 1.0);
}
