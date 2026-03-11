#include "VoxSceneResources.hpp"
#include "VkContext.hpp"

void VoxSceneResources::init(VkDevice device, VmaAllocator allocator, const VoxScene &scene) {
    m_allocator = allocator;

    const auto &model = scene.models.back();

    VkBufferCreateInfo voxelBufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = model.size.x * model.size.y * model.size.z + sizeof(GpuModelHeader),
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

    std::vector<uint8_t> voxelGrid(model.size.x * model.size.y * model.size.z, 0);
    for (const auto &voxel: model.voxels) {
        voxelGrid[voxel.coord.x + voxel.coord.y * model.size.x + voxel.coord.z * model.size.x * model.size.y] =
                voxel.colorIndex;
    }

    // Fill the voxel grid
    auto *mapped0 = static_cast<uint8_t *>(allocInfo[0].pMappedData);
    GpuModelHeader header{.size = model.size};
    memcpy(mapped0, &header, sizeof(GpuModelHeader));
    memcpy(mapped0 + sizeof(GpuModelHeader), voxelGrid.data(), voxelGrid.size());

    // Fill the color palette array
    auto *mapped1 = static_cast<glm::u8vec4 *>(allocInfo[1].pMappedData);
    memcpy(mapped1, scene.palette.data(), scene.palette.size() * sizeof(glm::u8vec4));
}

void VoxSceneResources::destroy() {
    vmaDestroyBuffer(m_allocator, buffer[0], allocation[0]);
    vmaDestroyBuffer(m_allocator, buffer[1], allocation[1]);
}
