#pragma once

#include <vk_mem_alloc.h>
#include "VkContext.hpp"

class Texture {
public:
    enum class Type {
        Storage,
        StorageBlit,
    };

    Texture(VkContext &ctx) : m_ctx(ctx) {}

    void init(uint32_t width, uint32_t height, VkFormat format, Type usage);
    void transitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout);
    void destroy();

    /* Getters */
    VkImage getImage() const { return m_image; }
    VkImageView getView() const { return m_view; }
    VkImageLayout getLayout() const { return m_currentLayout; }

private:
    VkImageUsageFlags getUsageFlags(Type usage);

    VkContext &m_ctx;
    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_view = VK_NULL_HANDLE;
    VkImageLayout m_currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VmaAllocation m_allocation = nullptr;
};
