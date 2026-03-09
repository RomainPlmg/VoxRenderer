#pragma once

#include <vk_mem_alloc.h>
#include "VkCommon.hpp"

class Camera;

struct CameraResources {
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocationInfo allocInfo{};
    VkDeviceAddress address = 0;

    void init(VkDevice device, VmaAllocator allocator);
    void update(const Camera &camera);
    void destroy();

private:
    VmaAllocator m_allocator = VK_NULL_HANDLE;
};
