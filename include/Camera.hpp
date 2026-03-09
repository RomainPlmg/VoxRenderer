#pragma once

#include <glm/glm.hpp>

struct CameraSettings {
    glm::vec3 position = glm::vec3(0.0f);
    float fov = 45.0f;
    float sensitivity = 100.0f;
};

struct CameraUniforms {
    glm::mat4 invView = glm::mat4(1.0f);
    glm::mat4 invProj = glm::mat4(1.0f);
};

class Camera {
public:
    explicit Camera(const CameraSettings &settings);

    void setPosition(const glm::vec3 position) { m_settings.position = position; }
    [[nodiscard]] glm::vec3 front() const { return m_front; }
    [[nodiscard]] glm::vec3 right() const { return m_right; }
    [[nodiscard]] glm::vec3 up() const { return m_up; }
    [[nodiscard]] glm::mat4 invView() const { return m_invView; }
    [[nodiscard]] glm::mat4 invProj() const { return m_invProj; }
    [[nodiscard]] const CameraSettings &getSettings() const { return m_settings; }

    void update(float ts);

private:
    glm::vec3 m_front = glm::vec3(0.0f);
    glm::vec3 m_up = glm::vec3(0.0f);
    glm::vec3 m_right = glm::vec3(0.0f);
    float m_yaw = 0.0f; // Rotation left/right
    float m_pitch = 0.0f; // Rotation up/down

    glm::vec2 m_lastMousePos = glm::vec2(0.0f);

    glm::mat4 m_invView = glm::mat4(1.0f);
    glm::mat4 m_invProj = glm::mat4(1.0f);

    CameraSettings m_settings;

    void updateVectors();
    void updateMatrices();
};
