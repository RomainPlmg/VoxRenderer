#include "Input.hpp"

#include <iostream>

#include "VkContext.hpp"
#include "Window.hpp"

GLFWwindow *Input::m_handle = nullptr;

void Input::init(const Window &window) {
    m_handle = window.getHandle();
    assert(m_handle);
}

bool Input::isKeyPressed(int keycode) {
    assert(m_handle);
    return glfwGetKey(m_handle, keycode) == GLFW_PRESS;
}

bool Input::isMouseButtonPressed(int button) {
    assert(m_handle);
    return glfwGetMouseButton(m_handle, button) == GLFW_PRESS;
}

glm::dvec2 Input::getMousePosition() {
    assert(m_handle);
    glm::dvec2 pos;
    glfwGetCursorPos(m_handle, &pos.x, &pos.y);
    return pos;
}

void Input::setCursorMode(const CursorMode mode) {
    int glfw_mode = GLFW_CURSOR_NORMAL;

    switch (mode) {
        case CursorMode::Normal:
            glfw_mode = GLFW_CURSOR_NORMAL;
            break;
        case CursorMode::Hidden:
            glfw_mode = GLFW_CURSOR_HIDDEN;
            break;
        case CursorMode::Disabled:
            glfw_mode = GLFW_CURSOR_DISABLED;
            break;
        default:
            std::cerr << "Unknown cursor mode." << std::endl;
            return;
    }

    glfwSetInputMode(m_handle, GLFW_CURSOR, glfw_mode);
}
