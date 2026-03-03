#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>

struct GLFWwindow;

struct WindowSpecification {
    std::string title;
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    bool is_resizable = true;
    bool vsync = true;
};

class Window {
public:
    explicit Window(const WindowSpecification &specification = WindowSpecification());
    ~Window();

    void init();
    void destroy();

    [[nodiscard]] bool shouldClose() const;
    [[nodiscard]] glm::vec2 getFrameBufferSize() const;
    [[nodiscard]] GLFWwindow *getHandle() const { return m_handle; }

private:
    WindowSpecification m_specification;
    GLFWwindow *m_handle = nullptr;
};
