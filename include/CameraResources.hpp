#pragma once

#include <memory>
#include <vk_mem_alloc.h>
#include "Buffer.hpp"

class Camera;

struct CameraResources {
    std::unique_ptr<StorageBuffer> buffer;
    // VkBuffer buffer = VK_NULL_HANDLE;
    // VmaAllocation allocation = VK_NULL_HANDLE;
    // VmaAllocationInfo allocInfo{};
    // VkDeviceAddress address = 0;

    void init(VkDevice device, VmaAllocator allocator);
    void update(const Camera &camera);
    void destroy();

private:
    VmaAllocator m_allocator = VK_NULL_HANDLE;
};
