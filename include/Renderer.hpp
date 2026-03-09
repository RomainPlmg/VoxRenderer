#pragma once

#include "VkContext.hpp"

class Renderer {
public:
    explicit Renderer(VkContext &ctx) : m_ctx(ctx) {}

    bool init(uint32_t width, uint32_t height);
    void render(VkCommandBuffer cmd);
    void shutdown();

    VkImage storageImage() const { return m_storageImage; }
    VkImageView storageImageView() const { return m_storageImageView; }
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

private:
    void createStorageImage(uint32_t width, uint32_t height);

    VkContext &m_ctx;
    VkImage m_storageImage;
    VkImageView m_storageImageView;
    VmaAllocation m_storageAllocation;

    uint32_t m_width = 0;
    uint32_t m_height = 0;
};
