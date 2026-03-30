#include "types.glsl"
#include "utils.glsl"
#include "buffers.glsl"

HitInfo ddaRayCollision(Ray ray, VoxelGridData voxelGrid) {
    HitInfo hitInfo;
    hitInfo.didHit = false;

    // DDA variables
    vec3 gridPos = floor(ray.origin);
    vec3 stepDir = sign(ray.dir);
    vec3 tDist = abs(1.0/(ray.dir + EPSILON));

    vec3 nextBoundary = gridPos + max(stepDir, vec3(0.0));
    vec3 tMax = abs((nextBoundary - ray.origin) / ray.dir);

    for (uint i = 0; i < 1000; i++) {
        vec3 stepMask = vec3(
            float(tMax.x <= min(tMax.y, tMax.z)),
            float(tMax.y < min(tMax.x, tMax.z)),
            float(tMax.z < min(tMax.x, tMax.y))
        );

        tMax += stepMask * tDist;
        gridPos += vec3(stepMask) * stepDir;

        // Check if position is in the model volume
        if (any(lessThan(gridPos, vec3(-EPSILON))) || 
            any(greaterThanEqual(gridPos, voxelGrid.size.xyz))) {
            break;
        }

        uint voxelIdx = uint(gridPos.x) +
                        uint(gridPos.y) * voxelGrid.size.x +
                        uint(gridPos.z) * voxelGrid.size.x * voxelGrid.size.y;
        uint colorIdx = voxelGrid.data[voxelIdx];

        if (colorIdx != 0) {
            hitInfo.didHit = true;
            float dist = min(tMax.x, min(tMax.y, tMax.z)); 
            hitInfo.hitDistance = dist;
            hitInfo.hitPoint = ray.origin + ray.dir * dist;
            hitInfo.hitNormal = calculateNormal(stepMask, stepDir);
            hitInfo.colorIdx = colorIdx;
            break;
        }
    }

    return hitInfo;
}