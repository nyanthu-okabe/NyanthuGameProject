#include "nyanchu/renderer_metal.h"
#include "platform/platform_utils.h"
#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

// Forward declare for getting NSWindow from GLFW
extern "C" {
    id glfwGetCocoaWindow(GLFWwindow* window);
}

namespace nyanchu {

struct Vertex {
    glm::vec4 pos;
    glm::vec4 color;
};

struct Uniforms {
    glm::mat4 mvp;
};

class RendererMetalImpl {
public:
    uint32_t _width, _height;

    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
    CAMetalLayer* _metalLayer;

    // Per-frame objects
    id<CAMetalDrawable> _drawable;
    id<MTLCommandBuffer> _commandBuffer;
    id<MTLRenderCommandEncoder> _commandEncoder;
    NSAutoreleasePool* _pool;
    id<MTLTexture> _depthTexture;

    // Triangle resources
    id<MTLRenderPipelineState> _trianglePipelineState;
    id<MTLBuffer> _triangleVertexBuffer;

    // Cube resources
    id<MTLRenderPipelineState> _cubePipelineState;
    id<MTLDepthStencilState> _depthState;
    id<MTLBuffer> _cubeVertexBuffer;
    id<MTLBuffer> _cubeIndexBuffer;
    id<MTLBuffer> _uniformBuffer;


    RendererMetalImpl(GLFWwindow* window, uint32_t width, uint32_t height) : _width(width), _height(height) {
        _device = MTLCreateSystemDefaultDevice();
        if (!_device) {
            std::cerr << "Metal is not supported on this device" << std::endl;
            return;
        }
        _commandQueue = [_device newCommandQueue];

        _metalLayer = [CAMetalLayer layer];
        _metalLayer.device = _device;
        _metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        _metalLayer.framebufferOnly = YES;
        _metalLayer.drawableSize = CGSizeMake(width, height);

        NSWindow* nswin = glfwGetCocoaWindow(window);
        NSView* view = [nswin contentView];
        [view setWantsLayer:YES];
        [view setLayer:_metalLayer];

        setupTriangle();
        setupCube();
        setupDepthBuffer();
    }

    void setupTriangle() {
        const char* shaderSrc = R"(
            using namespace metal;
            struct VertexIn { float4 position [[attribute(0)]]; float4 color [[attribute(1)]]; };
            struct VertexOut { float4 position [[position]]; float4 color; };
            vertex VertexOut vertex_main(const VertexIn in [[stage_in]]) { return { in.position, in.color }; }
            fragment float4 fragment_main(const VertexOut in [[stage_in]]) { return in.color; }
        )";

        NSError* error = nil;
        id<MTLLibrary> library = [_device newLibraryWithSource:[NSString stringWithUTF8String:shaderSrc] options:nil error:&error];
        id<MTLFunction> vertexFunc = [library newFunctionWithName:@"vertex_main"];
        id<MTLFunction> fragmentFunc = [library newFunctionWithName:@"fragment_main"];

        MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDescriptor.vertexFunction = vertexFunc;
        pipelineDescriptor.fragmentFunction = fragmentFunc;
        pipelineDescriptor.colorAttachments[0].pixelFormat = _metalLayer.pixelFormat;
        pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

        MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat4; // pos
        vertexDescriptor.attributes[0].offset = 0;
        vertexDescriptor.attributes[0].bufferIndex = 0;
        vertexDescriptor.attributes[1].format = MTLVertexFormatFloat4; // color
        vertexDescriptor.attributes[1].offset = sizeof(glm::vec4);
        vertexDescriptor.attributes[1].bufferIndex = 0;
        vertexDescriptor.layouts[0].stride = sizeof(Vertex);
        pipelineDescriptor.vertexDescriptor = vertexDescriptor;

        _trianglePipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];

        static const Vertex vertices[] = {
            {{ -0.5f, -0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }},
            {{  0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }},
            {{  0.0f,  0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }},
        };
        _triangleVertexBuffer = [_device newBufferWithBytes:vertices length:sizeof(vertices) options:MTLResourceStorageModeShared];
    }

    void setupCube() {
        const char* shaderSrc = R"(
            using namespace metal;
            struct VertexIn { float4 position [[attribute(0)]]; float4 color [[attribute(1)]]; };
            struct VertexOut { float4 position [[position]]; float4 color; };
            struct Uniforms { float4x4 mvp; };
            vertex VertexOut vertex_main(const VertexIn in [[stage_in]], constant Uniforms &uniforms [[buffer(1)]]) {
                VertexOut out;
                out.position = uniforms.mvp * in.position;
                out.color = in.color;
                return out;
            }
            fragment float4 fragment_main(const VertexOut in [[stage_in]]) { return in.color; }
        )";

        NSError* error = nil;
        id<MTLLibrary> library = [_device newLibraryWithSource:[NSString stringWithUTF8String:shaderSrc] options:nil error:&error];
        id<MTLFunction> vertexFunc = [library newFunctionWithName:@"vertex_main"];
        id<MTLFunction> fragmentFunc = [library newFunctionWithName:@"fragment_main"];

        MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDescriptor.vertexFunction = vertexFunc;
        pipelineDescriptor.fragmentFunction = fragmentFunc;
        pipelineDescriptor.colorAttachments[0].pixelFormat = _metalLayer.pixelFormat;
        pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

        MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat4;
        vertexDescriptor.attributes[0].offset = 0;
        vertexDescriptor.attributes[0].bufferIndex = 0;
        vertexDescriptor.attributes[1].format = MTLVertexFormatFloat4;
        vertexDescriptor.attributes[1].offset = sizeof(glm::vec4);
        vertexDescriptor.attributes[1].bufferIndex = 0;
        vertexDescriptor.layouts[0].stride = sizeof(Vertex);
        pipelineDescriptor.vertexDescriptor = vertexDescriptor;

        _cubePipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];

        static const Vertex vertices[] = {
            {{-1.0, -1.0, -1.0, 1.0}, {1, 0, 0, 1}}, {{ 1.0, -1.0, -1.0, 1.0}, {0, 1, 0, 1}},
            {{ 1.0,  1.0, -1.0, 1.0}, {0, 0, 1, 1}}, {{-1.0,  1.0, -1.0, 1.0}, {1, 1, 0, 1}},
            {{-1.0, -1.0,  1.0, 1.0}, {1, 0, 1, 1}}, {{ 1.0, -1.0,  1.0, 1.0}, {0, 1, 1, 1}},
            {{ 1.0,  1.0,  1.0, 1.0}, {1, 1, 1, 1}}, {{-1.0,  1.0,  1.0, 1.0}, {0, 0, 0, 1}}
        };
        _cubeVertexBuffer = [_device newBufferWithBytes:vertices length:sizeof(vertices) options:MTLResourceStorageModeShared];

        static const uint16_t indices[] = {
            0, 1, 2, 2, 3, 0, // Front
            1, 5, 6, 6, 2, 1, // Right
            5, 4, 7, 7, 6, 5, // Back
            4, 0, 3, 3, 7, 4, // Left
            3, 2, 6, 6, 7, 3, // Top
            4, 5, 1, 1, 0, 4  // Bottom
        };
        _cubeIndexBuffer = [_device newBufferWithBytes:indices length:sizeof(indices) options:MTLResourceStorageModeShared];
        _uniformBuffer = [_device newBufferWithLength:sizeof(Uniforms) options:MTLResourceStorageModeShared];
    }

    void setupDepthBuffer() {
        if (_width == 0 || _height == 0) return;
        MTLTextureDescriptor* depthTexDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                                width:_width
                                                                                               height:_height
                                                                                            mipmapped:NO];
        depthTexDesc.usage = MTLTextureUsageRenderTarget;
        depthTexDesc.storageMode = MTLStorageModePrivate;
        _depthTexture = [_device newTextureWithDescriptor:depthTexDesc];

        MTLDepthStencilDescriptor* depthDesc = [MTLDepthStencilDescriptor new];
        depthDesc.depthCompareFunction = MTLCompareFunctionLess;
        depthDesc.depthWriteEnabled = YES;
        _depthState = [_device newDepthStencilStateWithDescriptor:depthDesc];
    }

    ~RendererMetalImpl() {}

    void beginFrame() {
        _pool = [[NSAutoreleasePool alloc] init];
        _drawable = [_metalLayer nextDrawable];
        if (!_drawable) return;

        MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        if (!renderPassDescriptor) return;

        renderPassDescriptor.colorAttachments[0].texture = _drawable.texture;
        renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.1, 0.1, 0.1, 1.0);
        renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

        renderPassDescriptor.depthAttachment.texture = _depthTexture;
        renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
        renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
        renderPassDescriptor.depthAttachment.clearDepth = 1.0;

        _commandBuffer = [_commandQueue commandBuffer];
        _commandEncoder = [_commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        [_commandEncoder setDepthStencilState:_depthState];
    }

    void endFrame() {
        if (!_commandEncoder) return;
        [_commandEncoder endEncoding];
        [_commandBuffer presentDrawable:_drawable];
        [_commandBuffer commit];
        [_pool drain];
    }

    void resize(uint32_t width, uint32_t height) {
        _width = width;
        _height = height;
        _metalLayer.drawableSize = CGSizeMake(width, height);
        setupDepthBuffer();
    }

    void updateCubeUniforms(const glm::mat4& model) {
        if (_width == 0 || _height == 0) return;
        float aspect = (float)_width / (float)_height;
        glm::mat4 proj = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0, 0, -5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        Uniforms uniforms;
        uniforms.mvp = proj * view * model;
        memcpy([_uniformBuffer contents], &uniforms, sizeof(uniforms));
    }

    void drawTriangle() {
        if (!_commandEncoder) return;
        [_commandEncoder setRenderPipelineState:_trianglePipelineState];
        [_commandEncoder setVertexBuffer:_triangleVertexBuffer offset:0 atIndex:0];
        [_commandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    }

    void drawCube(const glm::mat4& modelMatrix) {
        if (!_commandEncoder) return;
        updateCubeUniforms(modelMatrix);
        [_commandEncoder setRenderPipelineState:_cubePipelineState];
        [_commandEncoder setVertexBuffer:_cubeVertexBuffer offset:0 atIndex:0];
        [_commandEncoder setVertexBuffer:_uniformBuffer offset:0 atIndex:1];
        [_commandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                      indexCount:36
                                       indexType:MTLIndexTypeUInt16
                                     indexBuffer:_cubeIndexBuffer
                               indexBufferOffset:0];
    }
};

RendererMetal::RendererMetal() : _impl(nullptr) {}
RendererMetal::~RendererMetal() { shutdown(); }

bool RendererMetal::initialize(GLFWwindow* window, uint32_t width, uint32_t height) {
    _impl = new RendererMetalImpl(window, width, height);
    return _impl != nullptr && _impl->_device != nil;
}

void RendererMetal::shutdown() { if (_impl) { delete _impl; _impl = nullptr; } }
void RendererMetal::beginFrame() { if (_impl) _impl->beginFrame(); }
void RendererMetal::endFrame() { if (_impl) _impl->endFrame(); }
void RendererMetal::resize(uint32_t width, uint32_t height) { if (_impl) _impl->resize(width, height); }
void RendererMetal::drawMesh(const char* meshName) { std::cout << "Drawing mesh: " << meshName << std::endl; }
void RendererMetal::drawTriangle() { if (_impl) _impl->drawTriangle(); }
void RendererMetal::drawCube(const glm::mat4& modelMatrix) { if (_impl) _impl->drawCube(modelMatrix); }

} // namespace nyanchu
