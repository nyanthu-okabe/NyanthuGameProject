#include "nyanchu/input.h"

namespace nyanchu {

// Store the instance per window.
static Input* s_instance = nullptr;

Input::Input(GLFWwindow* window) : m_window(window) {
    s_instance = this; // Singleton-like for callbacks. Assumes one window.
    m_currentKeys.fill(false);
    m_previousKeys.fill(false);
    m_currentMouseButtons.fill(false);
    m_previousMouseButtons.fill(false);
    m_currentMousePos = {0.0f, 0.0f};
    m_previousMousePos = {0.0f, 0.0f};

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPositionCallback);
    glfwSetWindowUserPointer(window, this); // Store pointer to this instance
}

void Input::update() {
    m_previousKeys = m_currentKeys;
    m_previousMouseButtons = m_currentMouseButtons;
    m_previousMousePos = m_currentMousePos;
    
    // For non-callback based state checking, you could poll here.
    // e.g. for IsKeyDown. Callbacks are better for pressed/released.
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


// Static callbacks that forward to the instance
void Input::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (input) {
        if (action == GLFW_PRESS) {
            input->m_currentKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            input->m_currentKeys[key] = false;
        }
    }
}

void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (input) {
        if (action == GLFW_PRESS) {
            input->m_currentMouseButtons[button] = true;
        } else if (action == GLFW_RELEASE) {
            input->m_currentMouseButtons[button] = false;
        }
    }
}

void Input::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (input) {
        input->m_currentMousePos.x = static_cast<float>(xpos);
        input->m_currentMousePos.y = static_cast<float>(ypos);
    }
}

} // namespace nyanchu
