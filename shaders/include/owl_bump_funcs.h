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
vec3 rotateVector(vec3 src, vec3 tgt, vec3 vec) {
    float angle = acos(dot(src,tgt));

    // Check for the case when src and tgt are the same vector, in which case
    // the cross product will be ill defined.
    if (angle == 0.0) {
        return vec;
    }
    vec3 axis = normalize(cross(src,tgt));
    mat4 R = rotationMatrix(axis,angle);

    // Rotate the vec by this rotation matrix
    vec4 _norm = R*vec4(vec,1.0);
    return _norm.xyz / _norm.w;
}

/**
  * Perturb the input normal by a single parametric value using a spiral formula.
  */
vec3 perturbNormalSpiral(vec3 normal, float t) {
    // Use a spiral formula to determine the 2D perturbation applied to our vertex normal
    float a = 0.3;   // Determines overall radius of spiral
    float b = 1000.0; // Determines frequency of spin
    float u = a * t * cos(b * t);
    float v = a * t * sin(b * t);

    // Find the rotation matrix to get from the vector [0,0,1] to our perturbed normal
    return rotateVector(vec3(0.0,0.0,1.0), normalize(vec3(u,v,1.0)), normal);
}

/**
  * Perturn a normal by using an input rotation vector
  */
vec3 perturbNormalVector(vec3 normal, vec3 pvec) {
    return rotateVector(vec3(0.0,0.0,1.0), pvec, normal);
}
