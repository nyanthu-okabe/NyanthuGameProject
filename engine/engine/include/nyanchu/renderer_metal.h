#pragma once

#include "renderer.h"

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
    void shutdown() override;

    void beginFrame() override;
    void endFrame() override;

    void drawMesh(const char* meshName) override;
    void drawTriangle() override;
    void drawCube() override;

private:
    class RendererMetalImpl* _impl;
};

} // namespace nyanchu