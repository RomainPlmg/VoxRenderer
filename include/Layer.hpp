#pragma once

#include "VkContext.hpp"

class Event;
class Renderer;

class Layer {
public:
    virtual void onAttach([[maybe_unused]] VkContext &ctx, [[maybe_unused]] Renderer &renderer) {}
    virtual void onDetach() {}
    virtual void onEvent([[maybe_unused]] Event &event) {}
    virtual void onUpdate([[maybe_unused]] float ts) {}
    virtual void onRender([[maybe_unused]] VkCommandBuffer cmd, [[maybe_unused]] Renderer &renderer) {}
};
