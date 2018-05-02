#version 410 core

layout(location=0) out vec3 FragColor;
in vec2 TexCoords;

uniform float Zdepth;
uniform sampler3D bump;

const vec2 size = vec2(2.0,0.0);
const ivec3 off = ivec3(-1, 0, 1);

void main() 
{
    vec3 coord = vec3(TexCoords, Zdepth);
    float s11 = texture(bump, coord).w;

    float s01 = textureOffset(bump, coord, off.xyy).w;
    float s21 = textureOffset(bump, coord, off.zyy).w;
    float s10 = textureOffset(bump, coord, off.yxy).w;
    float s12 = textureOffset(bump, coord, off.yzy).w;

    vec3 va = normalize(vec3(size.xy,s21-s01));
    vec3 vb = normalize(vec3(size.yx,s12-s10));

    FragColor = cross(va,vb);
}
