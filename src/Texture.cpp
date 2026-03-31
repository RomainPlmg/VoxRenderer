#include "Texture.hpp"

#include <cassert>

void Texture::init(uint32_t width, uint32_t height, VkFormat format, Type usage) {
    VkImageCreateInfo imgInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = {width, height, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = getUsageFlags(usage),
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo allocInfo{
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    assert(m_ctx.allocator() != VK_NULL_HANDLE && "VMA allocator not initialized");
    VK_CHECK(vmaCreateImage(m_ctx.allocator(), &imgInfo, &allocInfo, &m_image, &m_allocation, nullptr));

    VkImageViewCreateInfo viewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };

    VK_CHECK(vkCreateImageView(m_ctx.device(), &viewInfo, nullptr, &m_view));

    m_ctx.submitOneShot([&](VkCommandBuffer cmd) {
        transitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL);
    });
}

void Texture::transitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout) {
    VkAccessFlags srcAccess = 0;
    VkAccessFlags dstAccess = 0;
    VkPipelineStageFlags srcStage = 0;
    VkPipelineStageFlags dstStage = 0;

    switch (m_currentLayout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            srcAccess = 0;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        case VK_IMAGE_LAYOUT_GENERAL:
            srcAccess = VK_ACCESS_SHADER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            srcAccess = VK_ACCESS_TRANSFER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        default:
            assert(false && "Unsupported layout");
    }

    switch (newLayout) {
        case VK_IMAGE_LAYOUT_GENERAL:
            dstAccess = VK_ACCESS_SHADER_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            dstAccess = VK_ACCESS_TRANSFER_READ_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            dstAccess = 0;
            dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            break;
        default:
            assert(false && "Unsupported layout");
    }

    VkImageMemoryBarrier barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = srcAccess,
            .dstAccessMask = dstAccess,
            .oldLayout = m_currentLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_image,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    m_currentLayout = newLayout;
}

void Texture::destroy() {
    vkDestroyImageView(m_ctx.device(), m_view, nullptr);
    vmaDestroyImage(m_ctx.allocator(), m_image, m_allocation);
}

VkImageUsageFlags Texture::getUsageFlags(Type usage) {
    VkImageUsageFlags vkUsage = VK_IMAGE_USAGE_STORAGE_BIT;

    switch (usage) {
        case Type::StorageBlit:
            vkUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            break;
        default:
            break;
    }

    return vkUsage;
}
