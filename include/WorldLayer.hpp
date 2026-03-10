#pragma once

#include <filesystem>
#include <memory>

#include "Camera.hpp"
#include "CameraResources.hpp"
#include "ComputePass.hpp"
#include "Layer.hpp"
#include "VoxParser.hpp"

struct WorldLayerInfo {
    VkDeviceAddress cameraAddress = 0;
};

class WorldLayer : public Layer {
public:
    WorldLayer(const std::filesystem::path &voxFile);

    void onAttach(VkContext &ctx, Renderer &renderer) override;
    void onDetach() override;
    void onEvent(Event &event) override;
    void onUpdate(float ts) override;
    void onRender(VkCommandBuffer cmd, Renderer &renderer) override;

private:
    std::unique_ptr<ComputePass> m_svoPass;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<CameraResources> m_cameraResources;
    WorldLayerInfo m_info;

    VoxParser m_parser;
    VoxScene m_scene;
};
