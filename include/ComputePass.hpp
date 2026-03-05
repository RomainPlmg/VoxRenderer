#pragma once

#include "VkContext.hpp"

class ComputePass {
public:
    explicit ComputePass(VkContext &ctx) : m_ctx(ctx) {}

    bool init(const std::string &shaderPath, const std::vector<VkDescriptorSetLayoutBinding> &bindings);
    void bindImage(uint32_t binding, VkImageView view, VkImageLayout layout);
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
