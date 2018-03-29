#version 400

layout(triangles, equal_spacing, cw) in;

in vec3  tc_position[];
in vec3  tc_normal[];
in vec2  tc_uv[];

out vec3  te_position;
out vec3  te_normal;
out vec2  te_uv;

uniform mat4 MVP;

void main(void)
{
  te_normal   = (gl_TessCoord.x * tc_normal[0]   + gl_TessCoord.y * tc_normal[1]   + gl_TessCoord.z * tc_normal[2]);
  te_uv       = (gl_TessCoord.x * tc_uv[0]       + gl_TessCoord.y * tc_uv[1]       + gl_TessCoord.z * tc_uv[2]);
  te_position = (gl_TessCoord.x * tc_position[0] + gl_TessCoord.y * tc_position[1] + gl_TessCoord.z * tc_position[2]);
  gl_Position = MVP * vec4(te_position, 1.0);
}
