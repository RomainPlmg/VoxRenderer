#pragma once

#include <memory>

#include "Layer.hpp"
#include "Window.hpp"

class VkContext;

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

    static Application &get();
    static float getTime();

private:
    ApplicationSpecification m_specification;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<VkContext> m_vkContext;
    std::vector<std::unique_ptr<Layer>> m_layerStack;

    bool m_running = true;
};
