#pragma once

#include <vk_mem_alloc.h>
#include "VkContext.hpp"

class StorageBuffer {
public:
    StorageBuffer(VkDevice device, VmaAllocator allocator, size_t size);
    ~StorageBuffer();

    template<typename T>
    void emplace(T *data, uint32_t size, uint32_t offset = 0);
    void destroy();

    /* Getters */
    VkDeviceAddress address() const { return m_address; }

private:
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VmaAllocationInfo m_allocInfo{};
    VkDeviceAddress m_address = 0;

    VmaAllocator m_allocator;
};

template<typename T>
void StorageBuffer::emplace(T *data, uint32_t size, uint32_t offset) {
    auto *mapped = static_cast<uint8_t *>(m_allocInfo.pMappedData);
    memcpy(mapped + offset, data, size);
}
