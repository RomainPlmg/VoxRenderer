#include "VkContext.hpp"
#include <VkBootstrap.h>
#include "Logger.hpp"

bool VkContext::init(GLFWwindow *handle) {
    vkb::InstanceBuilder builder;

    // --- Instance
    auto inst_ret = builder.set_app_name("SVO Demo").request_validation_layers().use_default_debug_messenger().build();
    if (!inst_ret) {
        LOG_ERROR("Instance: {}.", inst_ret.error().message());
        return false;
    }
    m_instance = inst_ret.value();

    // --- Create the surface (GLFW)
    if (glfwCreateWindowSurface(m_instance, handle, nullptr, &m_surface) != VK_SUCCESS) {
        LOG_ERROR("Failed to create window surface.");
        return false;
    }

    // --- Physical device
    // Enable Vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features13{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE,
    };
    vkb::PhysicalDeviceSelector selector{m_instance};
    auto phys_ret = selector.set_surface(m_surface)
                            .set_minimum_version(1, 3)
                            .require_dedicated_transfer_queue()
                            .set_required_features_13(features13)
                            .select();
    if (!phys_ret) {
        LOG_ERROR("Physical device: {}.", phys_ret.error().message());
        for (const auto &e: phys_ret.detailed_failure_reasons())
            LOG_ERROR("{}.", e);

        return false;
    }

    vkb::DeviceBuilder device_builder{phys_ret.value()};

    // --- Logical device
    auto dev_ret = device_builder.build();
    if (!dev_ret) {
        LOG_ERROR("{}.", dev_ret.error().message());
        return false;
    }
    m_device = dev_ret.value();

    // -- Graphics & Present Queue
    m_graphicsQueue = m_device.get_queue(vkb::QueueType::graphics).value();
    m_presentQueue = m_device.get_queue(vkb::QueueType::present).value();

    // --- SwapChain
    int width, height;
    glfwGetFramebufferSize(handle, &width, &height);
    auto sc_ret = vkb::SwapchainBuilder{m_device}
                          .set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
                          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT) // Blit from compute shader
                          .set_desired_extent(width, height)
                          .build();
    if (!sc_ret) {
        LOG_ERROR("{}.", sc_ret.error().message());
        return false;
    }
    m_swapChain = sc_ret.value();

    // --- Command pool
    VkCommandPoolCreateInfo pool_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = m_device.get_queue_index(vkb::QueueType::graphics).value(),
    };
    VK_CHECK(vkCreateCommandPool(m_device, &pool_info, nullptr, &m_cmdPool));

    // --- Command buffers
    m_cmdBuffers.resize(FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_cmdPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = FRAMES_IN_FLIGHT,
    };
    VK_CHECK(vkAllocateCommandBuffers(m_device, &alloc_info, m_cmdBuffers.data()));

    // --- Sync primitives
    m_imageAvailable.resize(FRAMES_IN_FLIGHT);
    m_renderFinished.resize(FRAMES_IN_FLIGHT);
    m_inFlight.resize(FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo sem_info{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fen_info{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (int i = 0; i < FRAMES_IN_FLIGHT; ++i) {
        VK_CHECK(vkCreateSemaphore(m_device, &sem_info, nullptr, &m_imageAvailable[i]));
        VK_CHECK(vkCreateSemaphore(m_device, &sem_info, nullptr, &m_renderFinished[i]));
        VK_CHECK(vkCreateFence(m_device, &fen_info, nullptr, &m_inFlight[i]));
    }

    LOG_INFO("Vulkan initialized.");
    return true;
}

void VkContext::shutdown() {
    vkDeviceWaitIdle(m_device);

    for (int i = 0; i < FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(m_device, m_imageAvailable[i], nullptr);
        vkDestroySemaphore(m_device, m_renderFinished[i], nullptr);
        vkDestroyFence(m_device, m_inFlight[i], nullptr);
    }

    vkDestroyCommandPool(m_device, m_cmdPool, nullptr);
    vkb::destroy_swapchain(m_swapChain);
    vkb::destroy_device(m_device);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkb::destroy_instance(m_instance);
}

VkCommandBuffer VkContext::beginFrame() { return VkCommandBuffer{}; }

void VkContext::endFrame(VkCommandBuffer cmd) {}
