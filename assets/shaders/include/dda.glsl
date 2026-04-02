#ifndef DDA_GLSL
#define DDA_GLSL

#include "types.glsl"
#include "utils.glsl"
#include "buffers.glsl"

HitInfo ddaRayCollision(Ray ray, VoxelGridData voxelGrid, uint maxSteps) {
    HitInfo hitInfo;
    hitInfo.didHit = false;

    vec2 hit = intersectAABB(ray, voxelGrid.sceneMin.xyz, voxelGrid.sceneMax.xyz);
    if (hit.x > hit.y || hit.y < 0.0) { // No intersection
        return hitInfo;
    }

    // DDA constants
    const vec3 VOXELSPERUNIT = voxelGrid.size.xyz / (voxelGrid.sceneMax.xyz - voxelGrid.sceneMin.xyz);

    const vec3 safeDir = vec3(
        abs(ray.dir.x) < 1e-7 ? 1e-7 : ray.dir.x,
        abs(ray.dir.y) < 1e-7 ? 1e-7 : ray.dir.y,
        abs(ray.dir.z) < 1e-7 ? 1e-7 : ray.dir.z
    );
    const vec3 STEP = sign(safeDir);
    const vec3 DELTA = abs(1.0 / (safeDir * VOXELSPERUNIT));

    // DDA variables
    float tStart = max(hit.x + 1e-4, 0.0);
    vec3 localEntry = (ray.origin + ray.dir * tStart - voxelGrid.sceneMin.xyz) * VOXELSPERUNIT;
    vec3 gridPos = floor(localEntry);
    vec3 tMax = abs((gridPos - localEntry + max(STEP, vec3(0.0))) * DELTA);
    vec3 lastStepMask = vec3(0.0);

    // DDA traversal
    for (uint i = 0; i < maxSteps; i++) {
        // Check if position is in the model volume
        if (any(lessThan(gridPos, vec3(0.0))) || 
            any(greaterThanEqual(gridPos, voxelGrid.size.xyz))) {
            break;
        }

        uint voxelIdx = uint(gridPos.x) + uint(gridPos.y) * voxelGrid.size.x + uint(gridPos.z) * voxelGrid.size.x * voxelGrid.size.y;
        uint colorIdx = voxelGrid.data[voxelIdx];

        if (colorIdx != 0) {
            hitInfo.didHit = true;
            hitInfo.colorIdx = colorIdx;

            if (i == 0) {
                vec3 distanceToBoundary = abs(localEntry - round(localEntry));
                if (distanceToBoundary.x < min(distanceToBoundary.y, distanceToBoundary.z)) {
                    hitInfo.hitNormal = vec3(-STEP.x, 0, 0);
                }
                else if (distanceToBoundary.y < distanceToBoundary.z) {
                    hitInfo.hitNormal = vec3(0, -STEP.y, 0);
                }
                else {
                    hitInfo.hitNormal = vec3(0, 0, -STEP.z);
                }
            
                hitInfo.hitDistance = tStart;
            } else {
                hitInfo.hitNormal = -lastStepMask * STEP;
                hitInfo.hitDistance = tStart + dot(lastStepMask, tMax - DELTA);
            }

            hitInfo.hitPoint = voxelGrid.sceneMin.xyz + (gridPos + vec3(0.5) + hitInfo.hitNormal * 0.5) / VOXELSPERUNIT;
            break;
        }

        vec3 stepMask = vec3(
            float(tMax.x <= min(tMax.y, tMax.z)),
            float(tMax.y < min(tMax.x, tMax.z)),
            float(tMax.z < min(tMax.x, tMax.y))
        );

        lastStepMask = stepMask;
        tMax += stepMask * DELTA;
        gridPos += stepMask * STEP;
    }

    return hitInfo;
}

#endif // DDA_GLSL