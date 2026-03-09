#include "CameraResources.hpp"
#include "Camera.hpp"
#include "VkContext.hpp"

void CameraResources::init(VkDevice device, VmaAllocator allocator) {
    m_allocator = allocator;
    VkBufferCreateInfo bufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = sizeof(CameraUniforms),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
    };

    VmaAllocationCreateInfo allocCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
    };

    VK_CHECK(vmaCreateBuffer(m_allocator, &bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocInfo));

    VkBufferDeviceAddressInfo addrInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = buffer,
    };
    address = vkGetBufferDeviceAddress(device, &addrInfo);
}

void CameraResources::update(const Camera &camera) {
    auto *mapped = static_cast<CameraUniforms *>(allocInfo.pMappedData);
    mapped->invView = camera.invView();
    mapped->invProj = camera.invProj();
}

void CameraResources::destroy() { vmaDestroyBuffer(m_allocator, buffer, allocation); }
