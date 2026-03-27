#pragma once

#include <vk_mem_alloc.h>
#include "VoxParser.hpp"

#include "Buffer.hpp"

struct GpuModelHeader {
    glm::uvec4 size = glm::uvec4(0);
};

struct GpuMaterial {
    uint32_t type;
    float weight;
    float rough;
    float spec;
    float ior;
    float att;
    float flux;
    float _pad;
};

struct VoxSceneResources {
    std::unique_ptr<StorageBuffer> voxelBuffer;
    std::unique_ptr<StorageBuffer> paletteBuffer;
    std::unique_ptr<StorageBuffer> materialBuffer;

    void init(VkDevice device, VmaAllocator allocator, const VoxScene &scene);
    void destroy();

private:
    VmaAllocator m_allocator = VK_NULL_HANDLE;
};
