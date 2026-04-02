// ===========================
// Utility functions
// ===========================
vec2 intersectAABB(Ray ray, vec3 aabbMin, vec3 aabbMax) {
    vec3 t0 = (aabbMin - ray.origin) / ray.dir;
    vec3 t1 = (aabbMax - ray.origin) / ray.dir;
    // If negative direction, t0 > t1
    vec3 tMin = min(t0, t1);
    vec3 tMax = max(t0, t1);
    // In the box if we are in all 3 axis
    float tEnter = max(tMin.x, max(tMin.y, tMin.z));
    float tExit = min(tMax.x, min(tMax.y, tMax.z));

    return vec2(tEnter, tExit);
}

vec4 unpackRGBA(uint packedColor) {
    return vec4(
        float( packedColor        & 0xFFu) / 255.0, // Red
        float((packedColor >> 8)  & 0xFFu) / 255.0, // Green
        float((packedColor >> 16) & 0xFFu) / 255.0, // Blue
        float((packedColor >> 24) & 0xFFu) / 255.0  // Alpha
    );
}

vec3 calculateNormal(vec3 stepMask, vec3 stepDir) {
    if (stepMask.x > 0.0) {
        return vec3(-stepDir.x, 0.0, 0.0);
    } else if (stepMask.y > 0.0) {
        return vec3(0.0, -stepDir.y, 0.0);
    } else if (stepMask.z > 0.0) {
        return vec3(0.0, 0.0, -stepDir.z);
    }
}

vec3 getEnvironmentColour(Ray ray) {
    float skyGradient = ray.dir.y + 1;
    return mix(vec3(1.0), vec3(0.5, 0.7, 1.0), skyGradient);
}