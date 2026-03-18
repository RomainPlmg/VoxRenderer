#include "VoxSceneResources.hpp"
#include "VkContext.hpp"

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

// Recursive function to traverse the scene graph
void computeBoundingBox(uint32_t nodeId, SceneContext ctx, const VoxScene &scene, glm::ivec3 &sceneMin,
                        glm::ivec3 &sceneMax);
void fillGrid(uint32_t nodeId, SceneContext ctx, const VoxScene &scene, GridContext &gridContext);

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

    VkBufferCreateInfo voxelBufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = grid.size() * sizeof(uint32_t) + sizeof(GpuModelHeader),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
    };

    VkBufferCreateInfo paletteBufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = scene.palette.size() * sizeof(glm::u8vec4),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
    };

    VmaAllocationCreateInfo allocCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
    };

    VK_CHECK(vmaCreateBuffer(m_allocator, &voxelBufferInfo, &allocCreateInfo, &buffer[0], &allocation[0],
                             &allocInfo[0]));
    VK_CHECK(vmaCreateBuffer(m_allocator, &paletteBufferInfo, &allocCreateInfo, &buffer[1], &allocation[1],
                             &allocInfo[1]));

    VkBufferDeviceAddressInfo voxelAddrInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = buffer[0],
    };

    VkBufferDeviceAddressInfo paletteAddrInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = buffer[1],
    };

    address[0] = vkGetBufferDeviceAddress(device, &voxelAddrInfo);
    address[1] = vkGetBufferDeviceAddress(device, &paletteAddrInfo);

    // Fill the voxel grid
    auto *mapped0 = static_cast<uint8_t *>(allocInfo[0].pMappedData);
    GpuModelHeader header{.size = glm::uvec4(sceneSize, 0)};
    memcpy(mapped0, &header, sizeof(GpuModelHeader));
    memcpy(mapped0 + sizeof(GpuModelHeader), grid.data(), grid.size() * sizeof(uint32_t));

    // Fill the color palette array
    auto *mapped1 = static_cast<glm::u8vec4 *>(allocInfo[1].pMappedData);
    memcpy(mapped1, scene.palette.data(), scene.palette.size() * sizeof(glm::u8vec4));
}

void VoxSceneResources::destroy() {
    vmaDestroyBuffer(m_allocator, buffer[0], allocation[0]);
    vmaDestroyBuffer(m_allocator, buffer[1], allocation[1]);
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
        sceneMin = glm::min(sceneMin, ctx.translation);
        sceneMax = glm::max(sceneMax, ctx.translation + glm::ivec3(model.size));
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
        glm::ivec3 offset = ctx.translation - gridContext.sceneMin;
        for (const auto &voxel: model.voxels) {
            glm::ivec3 globalPos = glm::ivec3(voxel.coord) + offset;
            uint32_t index = globalPos.x + globalPos.y * gridContext.sceneSize.x +
                             globalPos.z * gridContext.sceneSize.x * gridContext.sceneSize.y;
            gridContext.grid[index] = voxel.colorIndex;
        }
    }
}
