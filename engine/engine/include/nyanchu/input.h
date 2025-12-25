#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <array>

namespace nyanchu {

// Re-map GLFW keys to a more generic enum if desired, but for now direct use is fine.
using Key = int;
using MouseButton = int;

class Input {
public:
    Input(GLFWwindow* window);

    void update();

    // Keyboard
    bool IsKeyDown(Key key);
    bool IsKeyPressed(Key key);
    bool IsKeyReleased(Key key);

    // Mouse
    glm::vec2 GetMousePosition();
    glm::vec2 GetMouseDelta();
    bool IsMouseButtonDown(MouseButton button);
    bool IsMouseButtonPressed(MouseButton button);
    bool isMouseButtonReleased(MouseButton button);

    // Callbacks need to be static to be used by GLFW
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    
private:
    GLFWwindow* m_window;

    std::array<bool, GLFW_KEY_LAST + 1> m_currentKeys;
    std::array<bool, GLFW_KEY_LAST + 1> m_previousKeys;

    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_currentMouseButtons;
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_previousMouseButtons;
    
    glm::vec2 m_currentMousePos;
    glm::vec2 m_previousMousePos;
};

} // namespace nyanchu
