#include "WorldLayer.hpp"
#include <memory>
#include <vector>
#include "ComputePass.hpp"
#include "Renderer.hpp"
#include "VkContext.hpp"

void WorldLayer::onAttach(VkContext &ctx, Renderer &renderer) {
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    bindings.emplace_back(VkDescriptorSetLayoutBinding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    });

    m_svoPass = std::make_unique<ComputePass>(ctx);
    m_svoPass->init(ASSETS_DIR "shaders/svo_trace.comp.spv", bindings);
    m_svoPass->bindImage(0, renderer.storageImageView(), VK_IMAGE_LAYOUT_GENERAL);
}

void WorldLayer::onDetach() { m_svoPass->shutdown(); }

void WorldLayer::onEvent([[maybe_unused]] Event &event) {}

void WorldLayer::onUpdate([[maybe_unused]] float ts) {}

void WorldLayer::onRender(VkCommandBuffer cmd, Renderer &renderer) {
    m_svoPass->dispatch(cmd, (renderer.width() + 7) / 8, (renderer.height() + 7) / 8);
}
