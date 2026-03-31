#pragma once

#include <memory>

#include "Texture.hpp"
#include "VkContext.hpp"

class Renderer {
public:
    explicit Renderer(VkContext &ctx) : m_ctx(ctx) {}

    bool init(uint32_t width, uint32_t height);
    void render(VkCommandBuffer cmd);
    void onResize(int width, int height);
    void shutdown();

    [[nodiscard]] const Texture& blitImage() const { return *m_blitTexture; }
    [[nodiscard]] uint32_t width() const { return m_width; }
    [[nodiscard]] uint32_t height() const { return m_height; }

private:
    VkContext &m_ctx;
    std::unique_ptr<Texture> m_blitTexture;

    uint32_t m_width = 0;
    uint32_t m_height = 0;
};
