#pragma once

#include "VkContext.hpp"

class Renderer {
public:
    explicit Renderer(VkContext &ctx) : m_ctx(ctx) {}

    bool init(uint32_t width, uint32_t height);
    void render(VkCommandBuffer cmd);
    void onResize(int width, int height);
    void shutdown();

    [[nodiscard]] VkImage storageImage() const { return m_storageImage; }
    [[nodiscard]] VkImageView storageImageView() const { return m_storageImageView; }
    [[nodiscard]] uint32_t width() const { return m_width; }
    [[nodiscard]] uint32_t height() const { return m_height; }

private:
    void createStorageImage(uint32_t width, uint32_t height);

    VkContext &m_ctx;
    VkImage m_storageImage = VK_NULL_HANDLE;
    VkImageView m_storageImageView = VK_NULL_HANDLE;
    VmaAllocation m_storageAllocation = VK_NULL_HANDLE;

    uint32_t m_width = 0;
    uint32_t m_height = 0;
};
