/*
 * Nyanthu Okabe 2025-12-25
 *
 * This project uses NyanchuEngine
 * The engine is private
 *
 * Copyright (c) 2025 nyanthu.com
 * All rights reserved.
 *
 * Do not modify or copy without permission.
 */

#include "application.h"
#include <iostream>
#include <chrono>
#include "platform_utils.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


Application::Application()
{
    m_engine = std::make_unique<nyanchu::Engine>();
}

Application::~Application()
{
    // m_engine is a unique_ptr and will be automatically destroyed,
    // calling the Engine's destructor which now handles shutdown.
}

bool Application::initialize()
{
    m_engine->init();

    std::string executableDir = getExecutableDir();
    std::string modelPath = executableDir + "/materials/model(1).obj";

    m_engine->playBgm("materials/bgm.wav");
    m_mesh = std::make_unique<nyanchu::Mesh>(modelPath.c_str());

    m_engine->cursor_disable();
    return true;
}

void Application::run()
{
    using clock = std::chrono::high_resolution_clock;
    auto lastFrameTime = clock::now();

    // For FPS calculation
    float timeAccumulator = 0.0f;

    float cameraSpeed = 2.5f;
    float mouseSensitivity = 0.1f;


    float frame = 0.0f;
    while (m_engine->isRunning())
    {
        auto currentTime = clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
        lastFrameTime = currentTime;

        m_engine->pollEvents();

        // --- Input and Camera Control ---
        {
            auto& input = m_engine->getInput();
            auto& camera = m_engine->getCamera();
            glm::vec3 front(camera.getFront().x, 0.0f, camera.getFront().z);
            front = glm::normalize(front); // 念のため正規化

            glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));

            if (input.IsKeyDown(GLFW_KEY_W))
                camera.MoveCamera(front * cameraSpeed * deltaTime);
            if (input.IsKeyDown(GLFW_KEY_S))
                camera.MoveCamera(-front * cameraSpeed * deltaTime);
            if (input.IsKeyDown(GLFW_KEY_A))
                camera.MoveCamera(-right * cameraSpeed * deltaTime);
            if (input.IsKeyDown(GLFW_KEY_D))
                camera.MoveCamera(right * cameraSpeed * deltaTime);


            if (input.IsKeyDown(GLFW_KEY_Q))
                m_engine->cursor_able();

            // Rotation
            glm::vec2 mouseDelta = input.GetMouseDelta();
            if (glm::length(mouseDelta) > 0.01f) {
                 camera.RotateCameraYaw(mouseDelta.x * mouseSensitivity);
                 camera.RotateCameraPitch(-mouseDelta.y * mouseSensitivity); // Inverted Y
            }
        }
        // --- End Input and Camera Control ---

        m_engine->beginFrame();

        // --- Drawing ---
        {
            // The object now stays at the origin, the camera moves around it
            glm::mat4 model = glm::mat4(1.0f);
            m_engine->getRenderer().drawMesh(*m_mesh, model);

            // Draw a cube slightly offset to see it
            model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 4.0f));
            m_engine->getRenderer().drawCube(model);

        }
        // --- End Drawing ---

        m_engine->endFrame();
    }
}
