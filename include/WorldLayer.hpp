#pragma once

#include <memory>
#include "ComputePass.hpp"
#include "Layer.hpp"

class WorldLayer : public Layer {
public:
    void onAttach(VkContext& ctx, Renderer &renderer) override;
    void onDetach() override;
    void onEvent(Event &event) override;
    void onUpdate(float ts) override;
    void onRender(VkCommandBuffer cmd, Renderer &renderer) override;

private:
    std::unique_ptr<ComputePass> m_svoPass;
};
