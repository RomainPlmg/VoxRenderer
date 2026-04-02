#include "CameraResources.hpp"
#include "Camera.hpp"

void CameraResources::init(VkDevice device, VmaAllocator allocator) {
    m_allocator = allocator;
    buffer = std::make_unique<StorageBuffer>(device, allocator, sizeof(CameraUniforms));
}

void CameraResources::update(const Camera &camera) {
    CameraUniforms uniform{
            .invView = camera.invView(),
            .invProj = camera.invProj(),
            .position = glm::vec4(camera.getSettings().position, 1.0f),
    };

    buffer->emplace(&uniform, sizeof(CameraUniforms));
}

void CameraResources::destroy() { buffer->destroy(); }
