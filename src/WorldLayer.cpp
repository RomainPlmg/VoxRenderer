#include "WorldLayer.hpp"
#include <memory>
#include <vector>
#include "ComputePass.hpp"
#include "Renderer.hpp"
#include "VkContext.hpp"

WorldLayer::WorldLayer(const std::filesystem::path &voxFile) { m_parser.parse(voxFile, m_scene); }

void WorldLayer::onAttach(VkContext &ctx, Renderer &renderer) {
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    bindings.emplace_back(VkDescriptorSetLayoutBinding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    });

    CameraSettings camSettings{
            .position = glm::vec3(0.0f, 0.0f, -50.0f),
            .sensitivity = 5.0f,
    };

    m_svoPass = std::make_unique<ComputePass>(ctx);
    m_camera = std::make_unique<Camera>(camSettings);
    m_cameraResources = std::make_unique<CameraResources>();
    m_sceneResources = std::make_unique<VoxSceneResources>();

    m_cameraResources->init(ctx.device(), ctx.allocator());
    m_sceneResources->init(ctx.device(), ctx.allocator(), m_scene);

    m_info.cameraAddress = m_cameraResources->address;
    m_info.voxelGridAddress = m_sceneResources->address[0];
    m_info.paletteAddress = m_sceneResources->address[1];

    m_svoPass->init(ASSETS_DIR "shaders/svo_trace.comp.spv", bindings);
    m_svoPass->bindImage(0, renderer.storageImageView(), VK_IMAGE_LAYOUT_GENERAL);
}

void WorldLayer::onDetach() {
    m_cameraResources->destroy();
    m_sceneResources->destroy();
    m_svoPass->shutdown();
}

void WorldLayer::onEvent([[maybe_unused]] Event &event) {}

void WorldLayer::onUpdate([[maybe_unused]] float ts) {
    m_camera->update(ts);
    m_cameraResources->update(*m_camera);
}

void WorldLayer::onRender(VkCommandBuffer cmd, Renderer &renderer) {
    m_svoPass->pushConstants<WorldLayerInfo>(cmd, m_info);
    m_svoPass->dispatch(cmd, (renderer.width() + 7) / 8, (renderer.height() + 7) / 8);
}
