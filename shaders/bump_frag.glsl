#version 430
uniform sampler2D NormalTexture;
uniform sampler2D ColourTexture;

// Set this property to change the scaling of the texture over the surface (higher means that the texture is repeated more frequently)
uniform float texCoordScale = 1.0;

// The output colour which will be output to the framebuffer
layout (location=0) out vec4 fragColour;

// Structure for holding light parameters
struct LightInfo
{
    vec4 Position; // Light position in eye coords.
    vec3 La; // Ambient light intensity
    vec3 Ld; // Diffuse light intensity
    vec3 Ls; // Specular light intensity
};

// We'll have a single light in the scene with some default values
uniform LightInfo Light = LightInfo(
            vec4(2.0, 2.0, 10.0, 1.0),   // position
            vec3(0.2, 0.2, 0.2),        // La
            vec3(1.0, 1.0, 1.0),        // Ld
            vec3(1.0, 1.0, 1.0)         // Ls
            );

// The material properties of our object
struct MaterialInfo
{
    vec3 Ka; // Ambient reflectivity
    vec3 Kd; // Diffuse reflectivity
    vec3 Ks; // Specular reflectivity
    float Shininess; // Specular shininess factor
};

// The object has a material
uniform MaterialInfo Material = MaterialInfo(
            vec3(0.1, 0.1, 0.1),    // Ka
            vec3(1.0, 1.0, 1.0),    // Kd
            vec3(1.0, 1.0, 1.0),    // Ks
            10.0);                  // Shininess

// Attributes passed on from the vertex shader
smooth in vec3 WSVertexPosition;
smooth in vec3 WSVertexNormal;
smooth in vec2 WSTexCoord;

uniform vec3 camPos;

/** From http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
  */
mat4 rotationMatrix(vec3 axis, float angle)
{
    //axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

/**
  * Rotate a vector vec by using the rotation that transforms from src to tgt.
  */
vec3 rotateVector(vec3 src, vec3 tgt, vec3 vec)
{
    float angle = acos(dot(src,tgt));

    // Check for the case when src and tgt are the same vector, in which case
    // the cross product will be ill defined.
    if (angle == 0.0)
    {
        return vec;
    }
    vec3 axis = normalize(cross(src,tgt));
    mat4 R = rotationMatrix(axis,angle);

    // Rotate the vec by this rotation matrix
    vec4 _norm = R*vec4(vec,1.0);
    return _norm.xyz / _norm.w;
}

void main()
{
    // Calculate the normal (this is the expensive bit in Phong)
    vec3 n = normalize( WSVertexNormal );

    // Calculate the view vector
    vec3 v = normalize(vec3(camPos-WSVertexPosition));

    // Extract the normal from the normal map (rescale to [-1,1]
    vec3 tgt = normalize(texture(NormalTexture, WSTexCoord*texCoordScale).rgb * 2.0 - 1.0);

    // The source is just up in the Z-direction
    vec3 src = vec3(0.0, 0.0, 1.0);

    // Perturb the normal according to the target
    vec3 np = rotateVector(src, tgt, n);

    // Calculate the light vector
    vec3 s = normalize( vec3(Light.Position) - WSVertexPosition );

    // Reflect the light about the surface normal
    vec3 r = reflect( -s, np );

    // Compute the light from the ambient, diffuse and specular components
    vec3 lightColor = (
            Light.La * Material.Ka +
            Light.Ld * Material.Kd * max( dot(s, np), 0.0 ) +
            Light.Ls * Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Shininess ));

    vec3 texColor = texture(ColourTexture, WSTexCoord*texCoordScale).rgb;

    // Use the following shader for the correct value
    fragColour = vec4(texColor*lightColor,1.0);
}  



