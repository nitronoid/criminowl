#version 410 core

layout(location=0) out vec3 FragColor;
in vec2 vs_texCoords;

uniform float u_zDepth;
uniform sampler3D u_bumpMap;

const vec2 k_size = vec2(2.0,0.0);
const ivec3 k_offset = ivec3(-1, 0, 1);

void main() 
{
    vec3 coord = vec3(vs_texCoords, u_zDepth);
    float s11 = texture(u_bumpMap, coord).w;

    float s01 = textureOffset(u_bumpMap, coord, k_offset.xyy).w;
    float s21 = textureOffset(u_bumpMap, coord, k_offset.zyy).w;
    float s10 = textureOffset(u_bumpMap, coord, k_offset.yxy).w;
    float s12 = textureOffset(u_bumpMap, coord, k_offset.yzy).w;

    vec3 va = normalize(vec3(k_size.xy, s21-s01));
    vec3 vb = normalize(vec3(k_size.yx, s12-s10));

    FragColor = cross(va,vb);
}
