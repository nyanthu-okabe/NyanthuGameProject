#pragma once

#include <cstdint>

// Forward declare GLFWwindow
struct GLFWwindow;

#include <glm/glm.hpp>

#include "mesh.h"

namespace nyanchu {

// Abstract base class for renderers
class IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual bool initialize(GLFWwindow* window, uint32_t width, uint32_t height) = 0;
    virtual void shutdown() = 0;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    virtual void drawMesh(const Mesh& mesh, const glm::mat4& modelMatrix) = 0;
    virtual void drawTriangle() = 0;
    virtual void drawCube(const glm::mat4& modelMatrix) = 0;
    virtual void resize(uint32_t width, uint32_t height) = 0;
};

} // namespace nyanchu