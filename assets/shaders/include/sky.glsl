#ifndef SKY_GLSL
#define SKY_GLSL

vec3 getEnvironmentColour(Ray ray) {
    float skyGradient = ray.dir.y + 1;
    return mix(vec3(1.0), vec3(0.5, 0.7, 1.0), skyGradient);
}

#endif // SKY_GLSL