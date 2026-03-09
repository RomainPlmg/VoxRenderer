#include "VkContext.hpp"
#include "Logger.hpp"

// Callback for validation layers
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
        [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, [[maybe_unused]] void *pUserData) {
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        LOG_ERROR("Vulkan Validation: {}", pCallbackData->pMessage);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        LOG_WARN("Vulkan Validation: {}", pCallbackData->pMessage);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        LOG_INFO("Vulkan Validation: {}", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

bool VkContext::init(GLFWwindow *handle) {
    vkb::InstanceBuilder builder;

    // --- Instance
    auto inst_ret = builder.set_app_name("SVO Demo")
                            .require_api_version(1, 3, 0)
                            .request_validation_layers()
                            .set_debug_callback(debugCallback)
                            .build();
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

    VmaAllocatorCreateInfo vma_info{
            .physicalDevice = m_device.physical_device.physical_device,
            .device = m_device.device,
            .instance = m_instance.instance,
            .vulkanApiVersion = VK_API_VERSION_1_3,
    };

    VK_CHECK(vmaCreateAllocator(&vma_info, &m_allocator));

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

    vmaDestroyAllocator(m_allocator);

    vkb::destroy_device(m_device);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkb::destroy_instance(m_instance);
}

VkCommandBuffer VkContext::beginFrame() {
    auto frame = m_frame % FRAMES_IN_FLIGHT;
    // Waiting for the GPU to finish the last frame
    vkWaitForFences(m_device, 1, &m_inFlight[frame], VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &m_inFlight[frame]);

    // Recover swapchain image index
    vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailable[frame], VK_NULL_HANDLE, &m_imgIdx);

    // Reset the command buffer for the new frame
    auto cmd = m_cmdBuffers[frame];
    vkResetCommandBuffer(cmd, 0);

    // Start to registers commands
    VkCommandBufferBeginInfo begin{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(cmd, &begin);

    auto images = m_swapChain.get_images().value();
    VkImageMemoryBarrier barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = images[m_imgIdx],
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);

    return cmd;
}

void VkContext::endFrame(VkCommandBuffer cmd) {
    vkEndCommandBuffer(cmd);

    auto frame = m_frame % FRAMES_IN_FLIGHT;

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_imageAvailable[frame],
            .pWaitDstStageMask = &wait_stage,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &m_renderFinished[frame],
    };
    vkQueueSubmit(m_graphicsQueue, 1, &submit, m_inFlight[frame]);

    // Present
    VkPresentInfoKHR present{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_renderFinished[frame],
            .swapchainCount = 1,
            .pSwapchains = &m_swapChain.swapchain,
            .pImageIndices = &m_imgIdx,
    };
    vkQueuePresentKHR(m_presentQueue, &present);

    ++m_frame;
}

void VkContext::submitOneShot(std::function<void(VkCommandBuffer)> fn) {
    // Transition UNDEFINED → GENERAL
    VkCommandBufferAllocateInfo alloc{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_cmdPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
    };
    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(m_device, &alloc, &cmd);

    VkCommandBufferBeginInfo begin{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cmd, &begin);

    fn(cmd);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd,
    };
    vkQueueSubmit(m_graphicsQueue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_device, m_cmdPool, 1, &cmd);
}
