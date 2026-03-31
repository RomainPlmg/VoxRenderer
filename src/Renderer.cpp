#include "Renderer.hpp"

#include "Application.hpp"
#include "Events.hpp"

bool Renderer::init(uint32_t width, uint32_t height) {
    m_blitTexture = std::make_unique<Texture>(m_ctx);
    m_blitTexture->init(width, height, VK_FORMAT_R8G8B8A8_UNORM, Texture::Type::StorageBlit);
    //     createStorageImage(width, height);

    m_width = width;
    m_height = height;

    // Subscribe to the event bus
    Application::get().getEventBus().subscribe<WindowResizeEvent>(
            [this](const auto &e) { onResize(e.width, e.height); });

    return true;
}

void Renderer::render(VkCommandBuffer cmd) {
    auto swapImages = m_ctx.swapChain().get_images().value();
    auto swapImage = swapImages[m_ctx.imgIdx()];

    // Move the storage image to PRESENT_SRC
    VkImageMemoryBarrier toSrc{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_blitTexture->getImage(),
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };

    // Move swap chain image from PRESENT_SRC to DST_SOURCE (swapImage)
    VkImageMemoryBarrier toDst{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = swapImage,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };

    VkImageMemoryBarrier barriers[] = {toSrc, toDst};
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 2, barriers);

    // Blit image (copy from TRANSFERT_SRC to TRANSFERT_DST)
    VkImageBlit region{
            .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
            .srcOffsets = {{0, 0, 0}, {(int32_t) m_width, (int32_t) m_height, 1}},
            .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
            .dstOffsets = {{0, 0, 0}, {(int32_t) m_width, (int32_t) m_height, 1}},
    };
    vkCmdBlitImage(cmd, m_blitTexture->getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapImage,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_NEAREST);

    // Reset layout of swap image (TRANSFERT_DST to PRESENT_SRC)
    VkImageMemoryBarrier toPresent{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = 0,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = swapImage,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };

    // Reset layout of storage image (TRANSFERT_SRC to GENERAL)
    VkImageMemoryBarrier toGeneral{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_blitTexture->getImage(),
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };

    VkImageMemoryBarrier barriers2[] = {toPresent, toGeneral};
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
                         nullptr, 2, barriers2);
}

void Renderer::onResize(int width, int height) {
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(Application::get().getWindow().getHandle(), &width, &height);
        glfwWaitEvents();
    }

    m_width = width;
    m_height = height;

    vkDeviceWaitIdle(m_ctx.device());

    m_ctx.createSwapChain(width, height);

    m_blitTexture->destroy();
    m_blitTexture->init(width, height, VK_FORMAT_R8G8B8A8_UNORM, Texture::Type::StorageBlit);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = m_blitTexture->getView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    Application::get().getEventBus().publishSync(StorageImageRecreatedEvent{
            .imageView = m_blitTexture->getView(),
    });
}

void Renderer::shutdown() {
    m_blitTexture->destroy();
}
