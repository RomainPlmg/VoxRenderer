#include "Window.hpp"

#include <stdexcept>

#include "Application.hpp"
#include "Events.hpp"
#include "Logger.hpp"
#include "VkContext.hpp"

Window::Window(const WindowSpecification &specification) : m_specification(specification) {}

Window::~Window() { destroy(); }

void Window::init() {
    m_handle = glfwCreateWindow(m_specification.width, m_specification.height, m_specification.title.c_str(), nullptr,
                                nullptr);
    if (!m_handle) {
        throw std::runtime_error("Failed to create GLFW window.");
    }

    glfwSetWindowUserPointer(m_handle, this);

    glfwSetFramebufferSizeCallback(m_handle, framebufferResizeCallback);
    LOG_INFO("Window initialized.");
}

void Window::destroy() {
    if (m_handle) {
        glfwDestroyWindow(m_handle);
    }
    m_handle = nullptr;
}

bool Window::shouldClose() const { return glfwWindowShouldClose(m_handle); }

glm::vec2 Window::getFrameBufferSize() const {
    int width, height;
    glfwGetFramebufferSize(m_handle, &width, &height);
    return {width, height};
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    Application::get().getEventBus().publish(WindowResizeEvent{
            .width = width,
            .height = height,
    });
}
