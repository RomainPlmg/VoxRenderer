#ifndef AO_GLSL
#define AO_GLSL

#include "utils.glsl"
#include "types.glsl"

float estimateAO(vec3 origin, vec3 normal, VoxelGridData voxelGrid) {
    float attenuation = 0.0;
    uint seed = 0xdeadbeef;

    for (uint i = 0; i < NB_AO_RAYS; i++) {
        Ray aoRay;
        aoRay.origin = origin + normal * EPSILON;
        aoRay.dir = randomHemisphereDirection(normal, seed);
        HitInfo hitInfo = ddaRayCollision(aoRay, voxelGrid, 3);
        
        if (hitInfo.didHit) attenuation += 1.0;
    }

    return 1.2 - (attenuation / float(NB_AO_RAYS));
}

#endif // AO_GLSL