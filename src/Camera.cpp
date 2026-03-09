#include "Camera.hpp"

#include <glm/ext/matrix_transform.hpp>

#include "Application.hpp"
#include "GLFW/glfw3.h"
#include "Input.hpp"

Camera::Camera(CameraSettings &settings) : m_settings(settings) { m_lastMousePos = Input::getMousePosition(); }

void Camera::update(float ts) {
    if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
        Input::setCursorMode(Input::CursorMode::Disabled);
        glm::vec2 mousePos = Input::getMousePosition();
        glm::vec2 mouseOffset = (m_lastMousePos - mousePos) * m_settings.sensitivity * ts;
        m_yaw -= mouseOffset.x;
        m_pitch += mouseOffset.y;

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
