#version 410 core

layout(location=0) out vec4 FragColor;
in vec3 vs_localPos;

uniform sampler2D u_sphereMap;

const vec2 k_invAtan = vec2(0.1591, 0.3183);

vec2 sampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= k_invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = sampleSphericalMap(normalize(vs_localPos)); // make sure to normalize localPos
    vec3 color = texture(u_sphereMap, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}
