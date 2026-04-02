#include "VoxSceneResources.hpp"

/////////////////////////////////////////////
// Context structures for scene graph navigation
/////////////////////////////////////////////
struct SceneContext {
    glm::ivec3 translation = glm::ivec3(0);
    glm::mat3 rotation = glm::mat3(1.0f);
};

struct GridContext {
    std::vector<uint32_t> &grid;
    glm::uvec3 sceneSize;
    glm::ivec3 sceneMin;
};

void computeBoundingBox(uint32_t nodeId, SceneContext ctx, const VoxScene &scene, glm::ivec3 &sceneMin,
                        glm::ivec3 &sceneMax);
void fillGrid(uint32_t nodeId, SceneContext ctx, const VoxScene &scene, GridContext &gridContext);
GpuMaterial toGPUMaterial(const VoxMaterialProperty &material);

void VoxSceneResources::init(VkDevice device, VmaAllocator allocator, const VoxScene &scene) {
    m_allocator = allocator;

    glm::ivec3 sceneMin(INT_MAX);
    glm::ivec3 sceneMax(INT_MIN);
    computeBoundingBox(0, {}, scene, sceneMin, sceneMax);
    glm::uvec3 sceneSize = glm::uvec3(sceneMax - sceneMin);

    std::vector<uint32_t> grid(sceneSize.x * sceneSize.y * sceneSize.z, 0);
    GridContext gridContext{
            .grid = grid,
            .sceneSize = sceneSize,
            .sceneMin = sceneMin,
    };
    fillGrid(0, {}, scene, gridContext);

    std::array<GpuMaterial, 256> gpuMaterials;
    for (size_t i = 0; i < gpuMaterials.size(); i++) {
        gpuMaterials[i] = toGPUMaterial(scene.materials[i]);
    }

    voxelBuffer = std::make_unique<StorageBuffer>(device, m_allocator,
                                                  grid.size() * sizeof(uint32_t) + sizeof(GpuModelHeader));
    paletteBuffer = std::make_unique<StorageBuffer>(device, m_allocator, scene.palette.size() * sizeof(glm::u8vec4));
    materialBuffer = std::make_unique<StorageBuffer>(device, m_allocator, gpuMaterials.size() * sizeof(GpuMaterial));

    GpuModelHeader header{
            .size = glm::uvec4(sceneSize, 0),
            .sceneMin = glm::ivec4(sceneMin, 0),
            .sceneMax = glm::ivec4(sceneMax, 0),
    };
    voxelBuffer->emplace(&header, sizeof(GpuModelHeader));
    voxelBuffer->emplace(grid.data(), grid.size() * sizeof(uint32_t), sizeof(header));

    paletteBuffer->emplace(scene.palette.data(), scene.palette.size() * sizeof(uint32_t));

    materialBuffer->emplace(gpuMaterials.data(), gpuMaterials.size() * sizeof(GpuMaterial));
}

void VoxSceneResources::destroy() {
    voxelBuffer->destroy();
    paletteBuffer->destroy();
    materialBuffer->destroy();
}

void computeBoundingBox(uint32_t nodeId, SceneContext ctx, const VoxScene &scene, glm::ivec3 &sceneMin,
                        glm::ivec3 &sceneMax) {
    const auto &node = scene.nodes.at(nodeId);

    if (auto *trn = dynamic_cast<VoxTransformNode *>(node.get())) {
        ctx.translation += trn->frameAttributes[0].translation; // No animations, so index 0 is ok :)
        // TODO : Manage rotation
        computeBoundingBox(trn->childId, ctx, scene, sceneMin, sceneMax);

    } else if (auto *grp = dynamic_cast<VoxGroupNode *>(node.get())) {
        for (auto childId: grp->childrenIdx)
            computeBoundingBox(childId, ctx, scene, sceneMin, sceneMax);
    } else if (auto *shp = dynamic_cast<VoxShapeNode *>(node.get())) {
        const auto &model = scene.models[shp->modelId[0]];
        glm::ivec3 halfSize = glm::ivec3(model.size) / 2;

        sceneMin = glm::min(sceneMin, ctx.translation - halfSize);
        sceneMax = glm::max(sceneMax, ctx.translation - halfSize + glm::ivec3(model.size));
    }
}


void fillGrid(uint32_t nodeId, SceneContext ctx, const VoxScene &scene, GridContext &gridContext) {
    const auto &node = scene.nodes.at(nodeId);

    if (auto *trn = dynamic_cast<VoxTransformNode *>(node.get())) {
        ctx.translation += trn->frameAttributes[0].translation; // No animations, so index 0 is ok :)
        // TODO : Manage rotation
        fillGrid(trn->childId, ctx, scene, gridContext);

    } else if (auto *grp = dynamic_cast<VoxGroupNode *>(node.get())) {
        for (auto childId: grp->childrenIdx)
            fillGrid(childId, ctx, scene, gridContext);

    } else if (auto *shp = dynamic_cast<VoxShapeNode *>(node.get())) {
        const auto &model = scene.models[shp->modelId[0]];
        glm::ivec3 halfSize = glm::ivec3(model.size) / 2;
        glm::ivec3 offset = (ctx.translation - halfSize) - gridContext.sceneMin;
        for (const auto &voxel: model.voxels) {
            glm::ivec3 globalPos = glm::ivec3(voxel.coord) + offset;
            uint32_t index = globalPos.x + globalPos.y * gridContext.sceneSize.x +
                             globalPos.z * gridContext.sceneSize.x * gridContext.sceneSize.y;
            gridContext.grid[index] = voxel.colorIndex;
        }
    }
}

GpuMaterial toGPUMaterial(const VoxMaterialProperty &material) {
    GpuMaterial gpuMaterial{};

    if (material.type.has_value())
        gpuMaterial.type = static_cast<uint32_t>(material.type.value());
    if (material.weight.has_value())
        gpuMaterial.weight = material.weight.value();
    if (material.rough.has_value())
        gpuMaterial.rough = material.rough.value();
    if (material.spec.has_value())
        gpuMaterial.spec = material.spec.value();
    if (material.ior.has_value())
        gpuMaterial.ior = material.ior.value();
    if (material.att.has_value())
        gpuMaterial.att = material.att.value();
    if (material.flux.has_value())
        gpuMaterial.flux = material.flux.value();

    return gpuMaterial;
}
