#pragma once

#include <vk_mem_alloc.h>
#include "VkCommon.hpp"
#include "VoxParser.hpp"

struct GpuModelHeader {
    glm::uvec4 size = glm::uvec4(0);
};

struct VoxSceneResources {
    VkBuffer buffer[2] = {VK_NULL_HANDLE};
    VmaAllocation allocation[2] = {VK_NULL_HANDLE};
    VmaAllocationInfo allocInfo[2]{};
    VkDeviceAddress address[2] = {0};

    void init(VkDevice device, VmaAllocator allocator, const VoxScene &scene);
    void destroy();

private:
    VmaAllocator m_allocator = VK_NULL_HANDLE;
};
