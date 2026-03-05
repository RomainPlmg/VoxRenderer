#include "ComputePass.hpp"

#include "Utils.hpp"

bool ComputePass::init(const std::string &shaderPath, const std::vector<VkDescriptorSetLayoutBinding> &bindings) {
    VkDescriptorSetLayoutCreateInfo layout_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data(),
    };
    VK_CHECK(vkCreateDescriptorSetLayout(m_ctx.device(), &layout_info, nullptr, &m_descriptorSetLayout));

    VkPipelineLayoutCreateInfo pipeline_layout_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &m_descriptorSetLayout,
    };
    VK_CHECK(vkCreatePipelineLayout(m_ctx.device(), &pipeline_layout_info, nullptr, &m_pipelineLayout));

    // Read the shader
    auto code = readSpirV(shaderPath);

    VkShaderModuleCreateInfo module_info{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode = reinterpret_cast<const uint32_t *>(code.data()),
    };
    VkShaderModule shader_module;
    VK_CHECK(vkCreateShaderModule(m_ctx.device(), &module_info, nullptr, &shader_module));

    VkPipelineShaderStageCreateInfo stage{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = shader_module,
            .pName = "main", // Shader entry point
    };

    VkComputePipelineCreateInfo pipeline_info{
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = stage,
            .layout = m_pipelineLayout,
    };
    VK_CHECK(vkCreateComputePipelines(m_ctx.device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_pipeline));

    // After pipeline creation, destroy the shader module
    vkDestroyShaderModule(m_ctx.device(), shader_module, nullptr);

    // Descriptor pool
    std::vector<VkDescriptorPoolSize> pool_sizes;
    for (const auto &b: bindings) {
        pool_sizes.push_back({b.descriptorType, b.descriptorCount});
    }

    VkDescriptorPoolCreateInfo pool_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 1,
            .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
            .pPoolSizes = pool_sizes.data(),
    };
    VK_CHECK(vkCreateDescriptorPool(m_ctx.device(), &pool_info, nullptr, &m_descriptorPool));

    // Descriptor set
    VkDescriptorSetAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &m_descriptorSetLayout,
    };
    VK_CHECK(vkAllocateDescriptorSets(m_ctx.device(), &alloc_info, &m_descriptorSet));

    return true;
}

void ComputePass::bindImage(uint32_t binding, VkImageView view, VkImageLayout layout) {
    VkDescriptorImageInfo img_info{
            .imageView = view,
            .imageLayout = layout,
    };
    VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_descriptorSet,
            .dstBinding = binding,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .pImageInfo = &img_info,
    };
    vkUpdateDescriptorSets(m_ctx.device(), 1, &write, 0, nullptr);
}

void ComputePass::dispatch(VkCommandBuffer cmd, uint32_t x, uint32_t y) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
    vkCmdDispatch(cmd, x, y, 1);
}

void ComputePass::shutdown() {
    vkDestroyPipeline(m_ctx.device(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_ctx.device(), m_pipelineLayout, nullptr);
    vkDestroyDescriptorPool(m_ctx.device(), m_descriptorPool, nullptr); // Descriptor set is destroyed with the pool
    vkDestroyDescriptorSetLayout(m_ctx.device(), m_descriptorSetLayout, nullptr);
}
