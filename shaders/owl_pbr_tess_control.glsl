#version 400

layout(vertices = 3) out;

in vec3 v_position[];
in vec3 v_normal[];
in vec2 v_uv[];

out vec3 tc_position[];
out vec3 tc_normal[];
out vec2 tc_uv[];

uniform int innerTess = 16;
uniform int outerTess = 16;

#define ID gl_InvocationID

void main(void)
{
  tc_position[ID] = v_position[ID];
  tc_normal[ID] = v_normal[ID];
  tc_uv[ID] = v_uv[ID];

  if(ID == 0)
  {
    gl_TessLevelInner[0] = innerTess;
    gl_TessLevelOuter[0] = outerTess;
    gl_TessLevelOuter[1] = outerTess;
    gl_TessLevelOuter[2] = outerTess;
  }
}
