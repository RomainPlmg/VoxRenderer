#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>
#include <glm/glm.hpp>
#include <vector>

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

    [[nodiscard]] VkDevice device() const { return m_device; }
    vkb::Swapchain &swapChain() { return m_swapChain; }

private:
    // Core
    vkb::Instance m_instance;
    vkb::Device m_device;
    vkb::Swapchain m_swapChain;
    VkSurfaceKHR m_surface;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    // Sync
    VkCommandPool m_cmdPool;
    std::vector<VkCommandBuffer> m_cmdBuffers;
    std::vector<VkSemaphore> m_imageAvailable;
    std::vector<VkSemaphore> m_renderFinished;
    std::vector<VkFence> m_inFlight;
    uint32_t m_frame = 0;
    uint32_t m_imgIdx = 0;
};
