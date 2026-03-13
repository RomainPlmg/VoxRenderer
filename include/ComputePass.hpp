#pragma once

#include "VkContext.hpp"

class ComputePass {
public:
    explicit ComputePass(VkContext &ctx) : m_ctx(ctx) {}

    bool init(const std::string &shaderPath, const std::vector<VkDescriptorSetLayoutBinding> &bindings);
    void bindImage(uint32_t binding, VkImageView view, VkImageLayout layout);

    template<typename T>
    void pushConstants(VkCommandBuffer cmd, const T &data);

    void dispatch(VkCommandBuffer cmd, uint32_t x, uint32_t y);
    void shutdown();

private:
    VkContext &m_ctx;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
};

template<typename T>
void ComputePass::pushConstants(VkCommandBuffer cmd, const T &data) {
    static_assert(sizeof(T) <= 128, "Push constant exceeds guaranteed 128 bytes limit");
    vkCmdPushConstants(cmd, m_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(T), &data);
}
