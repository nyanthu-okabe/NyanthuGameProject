#pragma once

#include "renderer.h"
#include <memory>

// Forward declare GLFWwindow
struct GLFWwindow;

namespace nyanchu {

class RendererMetalImpl;

class RendererMetal : public IRenderer
{
public:
    RendererMetal();
    ~RendererMetal() override;

    bool initialize(GLFWwindow* window, uint32_t width, uint32_t height) override;
    void shutdown() override; // Keep for interface, but will be empty.

    void beginFrame(const Camera& camera) override;
    void endFrame() override;

    void drawMesh(const Mesh& mesh, const glm::mat4& modelMatrix) override;
    void drawTriangle() override;
    void drawCube(const glm::mat4& modelMatrix) override;
    void resize(uint32_t width, uint32_t height) override;

private:
    std::unique_ptr<RendererMetalImpl> _impl;
};

} // namespace nyanchu