#pragma once

#include "VkCommon.hpp"
#include <vk_mem_alloc.h>
#include <functional>

constexpr uint32_t FRAMES_IN_FLIGHT = 3;
#define VK_CHECK(x)                                                                                                    \
    do {                                                                                                               \
        VkResult r = (x);                                                                                              \
        assert(r == VK_SUCCESS && #x);                                                                                 \
    } while (0)

class VkContext {
public:
    bool init(GLFWwindow *handle);
    void shutdown();
    VkCommandBuffer beginFrame();
    void endFrame(VkCommandBuffer cmd);
    void submitOneShot(std::function<void(VkCommandBuffer)> fn);

    [[nodiscard]] VkDevice device() const { return m_device; }
    vkb::Swapchain &swapChain() { return m_swapChain; }
    VmaAllocator &allocator() { return m_allocator; }
    uint32_t imgIdx() const { return m_imgIdx; }

private:
    // Core
    vkb::Instance m_instance;
    vkb::Device m_device;
    vkb::Swapchain m_swapChain;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    VmaAllocator m_allocator = VK_NULL_HANDLE;

    // Sync
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_cmdBuffers;
    std::vector<VkSemaphore> m_imageAvailable;
    std::vector<VkSemaphore> m_renderFinished;
    std::vector<VkFence> m_inFlight;
    uint32_t m_frame = 0;
    uint32_t m_imgIdx = 0;
};
