#include "Camera.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Application.hpp"
#include "GLFW/glfw3.h"
#include "Input.hpp"

constexpr float VELOCITY = 15.0f;

Camera::Camera(const CameraSettings &settings) : m_settings(settings) { m_lastMousePos = Input::getMousePosition(); }

void Camera::update(float ts) {
    // TODO: Move camera controller elsewhere
    glm::vec3 frontXZ = glm::normalize(glm::vec3(m_front.x, 0.0f, m_front.z));
    glm::vec3 rightXZ = glm::normalize(glm::vec3(m_right.x, 0.0f, m_right.z));

    auto worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 dir{0.0f};
    if (Input::isKeyPressed(GLFW_KEY_D))
        dir += rightXZ;
    if (Input::isKeyPressed(GLFW_KEY_A))
        dir -= rightXZ;
    if (Input::isKeyPressed(GLFW_KEY_W))
        dir += frontXZ;
    if (Input::isKeyPressed(GLFW_KEY_S))
        dir -= frontXZ;
    if (Input::isKeyPressed(GLFW_KEY_SPACE))
        dir -= worldUp;
    if (Input::isKeyPressed(GLFW_KEY_LEFT_SHIFT))
        dir += worldUp;

    auto camPos = m_settings.position;
    camPos += dir * VELOCITY * ts;
    setPosition(camPos);

    if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
        Input::setCursorMode(Input::CursorMode::Disabled);
        glm::vec2 mousePos = Input::getMousePosition();
        glm::vec2 mouseOffset = (m_lastMousePos - mousePos) * m_settings.sensitivity * ts;
        m_yaw -= mouseOffset.x;
        m_pitch -= mouseOffset.y;

        // Make sure that when pitch is out of bounds, screen doesn't get
        // flipped
        if (m_pitch > 89.0f)
            m_pitch = 89.0f;
        if (m_pitch < -89.0f)
            m_pitch = -89.0f;
    } else {
        Input::setCursorMode(Input::CursorMode::Normal);
    }

    updateVectors();
    updateMatrices();

    m_lastMousePos = Input::getMousePosition();
}

void Camera::updateVectors() { // Calculate the new Front vector
    glm::vec3 front(0.0f);
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);

    auto worldUp = glm::vec3(0, 1, 0);
    m_right = glm::normalize(glm::cross(m_front, worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}
void Camera::updateMatrices() {
    auto size = Application::get().getFrameBufferSize();
    float aspect = size.x / size.y;

    glm::mat4 view = glm::lookAt(m_settings.position, m_settings.position + m_front, m_up);
    glm::mat4 proj;

    proj = glm::perspective(glm::radians(m_settings.fov), aspect, 0.1f, 100.0f);

    m_invView = glm::inverse(view);
    m_invProj = glm::inverse(proj);
}
