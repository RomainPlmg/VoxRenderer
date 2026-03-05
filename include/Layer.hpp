#pragma once

#include "VkContext.hpp"

class Event;
class Renderer;

class Layer {
public:
    virtual ~Layer() {
        onDetach();
    };

    virtual void onAttach(VkContext& ctx, Renderer& renderer) {}
    virtual void onDetach() {}
    virtual void onEvent(Event &event) {}
    virtual void onUpdate(float ts) {}
    virtual void onRender(VkCommandBuffer cmd, Renderer& renderer) {}
};
