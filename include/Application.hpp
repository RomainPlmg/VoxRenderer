#pragma once

#include <memory>

#include "Layer.hpp"
#include "Window.hpp"

class VkContext;
class Renderer;

struct ApplicationSpecification {
    std::string name = "Sparse Voxel Octree Engine";
    WindowSpecification window_spec;
};

class Application {
public:
    explicit Application(const ApplicationSpecification &specification = ApplicationSpecification());
    ~Application();

    void run();
    void stop();

    template<typename TLayer, typename... TArgs>
        requires(std::is_base_of_v<Layer, TLayer>)
    void pushLayer(TArgs &&...args) {
        m_layerStack.push_back(std::make_unique<TLayer>(std::forward<TArgs>(args)...));
    }

    [[nodiscard]] glm::vec2 getFrameBufferSize() const {
        assert(m_window);
        return m_window->getFrameBufferSize();
    }

    [[nodiscard]] Window &getWindow() const {
        assert(m_window);
        return *m_window;
    }

    [[nodiscard]] Renderer &getRenderer() const {
        assert(m_renderer);
        return *m_renderer;
    }

    static Application &get();
    static float getTime();

private:
    ApplicationSpecification m_specification;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<VkContext> m_vkContext;
    std::unique_ptr<Renderer> m_renderer;
    std::vector<std::unique_ptr<Layer>> m_layerStack;

    float m_accuTime = 0.0f;
    uint32_t m_frameCount = 0;

    bool m_running = true;
};
