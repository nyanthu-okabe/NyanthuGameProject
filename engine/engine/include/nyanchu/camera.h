#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace nyanchu {

class Camera {
public:
    Camera();

    // Setters
    void SetCameraPosition(const glm::vec3& pos);
    void MoveCamera(const glm::vec3& delta);
    void SetCameraTarget(const glm::vec3& target);
    void RotateCameraYaw(float angle);
    void RotateCameraPitch(float angle);
    void LookAt(const glm::vec3& target);
    void ZoomCamera(float delta);

    // Getters
    const glm::vec3& getPosition() const { return m_position; }
    const glm::vec3& getTarget() const { return m_target; }
    glm::mat4 getViewMatrix() const;
    const glm::vec3& getFront() const { return m_front; }
    const glm::vec3& getRight() const { return m_right; }

private:
    void updateVectors();

    glm::vec3 m_position;
    glm::vec3 m_target;
    glm::vec3 m_up;

    // Euler Angles
    float m_yaw;
    float m_pitch;

    // Camera vectors
    glm::vec3 m_front;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;
};

} // namespace nyanchu
