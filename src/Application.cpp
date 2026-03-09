#include "Application.hpp"

#include <GLFW/glfw3.h>

#include "Input.hpp"
#include "Logger.hpp"
#include "Renderer.hpp"
#include "VkContext.hpp"


static Application *s_application = nullptr;

static void glfwErrorCallback(int error, const char *description) {
    LOG_ERROR("[GLFW Error {}] -> {}", error, description);
}

Application::Application(const ApplicationSpecification &specification) : m_specification(specification) {
    s_application = this;

    Logger::init();

    glfwSetErrorCallback(glfwErrorCallback);
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Set window title to app name if empty
    if (m_specification.window_spec.title.empty()) {
        m_specification.window_spec.title = m_specification.name;
    }

    m_window = std::make_unique<Window>(m_specification.window_spec);
    m_vkContext = std::make_unique<VkContext>();
    m_renderer = std::make_unique<Renderer>(*m_vkContext);

    // Init window
    m_window->init();

    // Init Vulkan
    if (!m_vkContext->init(m_window->getHandle())) {
        throw std::runtime_error("Failed to init Vulkan backend graphics API.");
    }

    // Init renderer
    if (!m_renderer->init(m_specification.window_spec.width, m_specification.window_spec.height)) {
        throw std::runtime_error("Failed to init renderer.");
    }

    Input::init(*m_window);
}

Application::~Application() {
    m_layerStack.clear();
    m_renderer->shutdown();
    m_vkContext->shutdown();
    m_window->destroy();
    glfwTerminate();
    s_application = nullptr;

    LOG_INFO("Goodbye.");
}

void Application::run() {
    m_running = true;
    float last_time = getTime();

    // Attach all layers
    for (auto &layer: m_layerStack) {
        layer->onAttach(*m_vkContext, *m_renderer);
    }

    // Main application loop
    while (m_running) {
        glfwPollEvents();

        if (m_window->shouldClose()) {
            stop();
            break;
        }

        const float current_time = getTime();
        const float timestep = glm::clamp(current_time - last_time, 0.001f, 0.1f);
        last_time = current_time;

        // Main layer update here
        for (const auto &layer: m_layerStack) {
            layer->onUpdate(timestep);
        }

        auto cmd = m_vkContext->beginFrame(); // Start to render the frame

        // NOTE: rendering can be done elsewhere (e.g. render thread)
        for (const auto &layer: m_layerStack) {
            layer->onRender(cmd, *m_renderer);
        }

        m_renderer->render(cmd);

        m_vkContext->endFrame(cmd);
    }
}

void Application::stop() { m_running = false; }

Application &Application::get() {
    assert(s_application);
    return *s_application;
}

float Application::getTime() { return static_cast<float>(glfwGetTime()); }
