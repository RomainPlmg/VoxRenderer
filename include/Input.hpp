#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;
class Window;

class Input {
public:
    enum class CursorMode {
        Normal,
        Hidden,
        Disabled,
    };

    static void init(const Window &window);
    static bool isKeyPressed(int keycode);
    static bool isMouseButtonPressed(int button);
    static glm::dvec2 getMousePosition();
    static void setCursorMode(const CursorMode mode);

private:
    static GLFWwindow *m_handle;
};
