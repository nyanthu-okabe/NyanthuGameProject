#include "nyanchu/camera.h"
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace nyanchu {

Camera::Camera()
    : m_position(0.0f, 2.0f, 5.0f), // Start position
      m_target(0.0f, 0.0f, 0.0f),   // Look at origin
      m_worldUp(0.0f, 1.0f, 0.0f)
{
    m_front = glm::normalize(m_target - m_position);
    m_pitch = glm::degrees(asin(m_front.y));
    m_yaw = glm::degrees(atan2(m_front.z, m_front.x));
    updateVectors();
}

void Camera::SetCameraPosition(const glm::vec3& pos) {
    m_position = pos;
}

void Camera::MoveCamera(const glm::vec3& delta) {
    m_position += delta;
}

void Camera::SetCameraTarget(const glm::vec3& target) {
    m_target = target;
    m_front = glm::normalize(m_target - m_position);
    // Also update pitch and yaw
    m_pitch = glm::degrees(asin(m_front.y));
    m_yaw = glm::degrees(atan2(m_front.z, m_front.x));
    updateVectors();
}

void Camera::RotateCameraYaw(float angle) {
    m_yaw += angle;
    updateVectors();
}

void Camera::RotateCameraPitch(float angle) {
    m_pitch += angle;
    // Constrain pitch
    if (m_pitch > 89.0f) m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;
    updateVectors();
}

void Camera::LookAt(const glm::vec3& target) {
    SetCameraTarget(target);
}

void Camera::ZoomCamera(float delta) {
    m_position += m_front * delta;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

void Camera::updateVectors() {
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);
    // Also re-calculate the Right and Up vector
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));

    // Update target based on new orientation
    // This makes RotateCameraYaw/Pitch orbit around the camera's position
    // If you want to orbit around a target, the logic would be different
    // m_target = m_position + m_front;
}

} // namespace nyanchu
