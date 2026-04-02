#ifndef UTILS_GLSL
#define UTILS_GLSL

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

uint nextRandom(inout uint state) {
    state = state * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

// Return a value between 0 & 1;
float randomFloat(inout uint state) {
    return float(nextRandom(state)) / 4294967295.0;
}

vec3 randomDirection(inout uint seed) {
    for (int limit = 0; limit < 100; limit++) {
        // Generate random direction in a cube -1 <-> 1
        vec3 rndVector = vec3 (
            randomFloat(seed) * 2.0 - 1,
            randomFloat(seed) * 2.0 - 1,
            randomFloat(seed) * 2.0 - 1
        );

        // This method generate a cube, then the edges are denser in points, so need to filter
        // If the vector is in the sphere
        if (dot(rndVector, rndVector) < 1.0) {
            return normalize(rndVector);
        }
    }

    return vec3(0.0);
}

vec3 randomHemisphereDirection(vec3 normal, inout uint seed) {
    vec3 rndDir = randomDirection(seed);
    return rndDir * sign(dot(normal, rndDir));
}

#endif // UTILS_GLSL