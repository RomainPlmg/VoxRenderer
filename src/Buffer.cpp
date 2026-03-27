#include "Buffer.hpp"

// ===========================
// Storage Buffer
// ===========================
StorageBuffer::StorageBuffer(VkDevice device, VmaAllocator allocator, size_t size) : m_allocator(allocator) {
    VkBufferCreateInfo bufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
    };

    VmaAllocationCreateInfo allocCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
    };

    VK_CHECK(vmaCreateBuffer(m_allocator, &bufferInfo, &allocCreateInfo, &m_buffer, &m_allocation, &m_allocInfo));

    VkBufferDeviceAddressInfo addrInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = m_buffer,
    };

    m_address = vkGetBufferDeviceAddress(device, &addrInfo);
}

StorageBuffer::~StorageBuffer() {
    if (m_buffer != VK_NULL_HANDLE)
        destroy();
}

void StorageBuffer::destroy() {
    vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
    m_buffer = VK_NULL_HANDLE;
}