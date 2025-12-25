#include "nyanchu/input.h"
#include <GLFW/glfw3.h>

namespace nyanchu {

Input::Input(GLFWwindow* window) : m_window(window) {
    m_currentKeys.fill(false);
    m_previousKeys.fill(false);
    m_currentMouseButtons.fill(false);
    m_previousMouseButtons.fill(false);
    
    double x, y;
    glfwGetCursorPos(m_window, &x, &y);
    m_currentMousePos = glm::vec2(static_cast<float>(x), static_cast<float>(y));
    m_previousMousePos = m_currentMousePos;
}

void Input::update() {
    m_previousKeys = m_currentKeys;
    m_previousMouseButtons = m_currentMouseButtons;
    m_previousMousePos = m_currentMousePos;
    
    for (int key = 0; key <= GLFW_KEY_LAST; ++key) {
        m_currentKeys[key] = (glfwGetKey(m_window, key) == GLFW_PRESS);
    }
    for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; ++button) {
        m_currentMouseButtons[button] = (glfwGetMouseButton(m_window, button) == GLFW_PRESS);
    }
    double x, y;
    glfwGetCursorPos(m_window, &x, &y);
    m_currentMousePos = glm::vec2(static_cast<float>(x), static_cast<float>(y));
}

bool Input::IsKeyDown(Key key) {
    return m_currentKeys[key];
}

bool Input::IsKeyPressed(Key key) {
    return m_currentKeys[key] && !m_previousKeys[key];
}

bool Input::IsKeyReleased(Key key) {
    return !m_currentKeys[key] && m_previousKeys[key];
}

glm::vec2 Input::GetMousePosition() {
    return m_currentMousePos;
}

glm::vec2 Input::GetMouseDelta() {
    return m_currentMousePos - m_previousMousePos;
}

bool Input::IsMouseButtonDown(MouseButton button) {
    return m_currentMouseButtons[button];
}

bool Input::IsMouseButtonPressed(MouseButton button) {
    return m_currentMouseButtons[button] && !m_previousMouseButtons[button];
}

bool Input::isMouseButtonReleased(MouseButton button) {
    return !m_currentMouseButtons[button] && m_previousMouseButtons[button];
}

} // namespace nyanchu
