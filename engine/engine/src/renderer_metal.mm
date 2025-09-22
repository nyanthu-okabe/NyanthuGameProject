#include "nyanchu/renderer_metal.h"
#include "platform/platform_utils.h"
#include <iostream>

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

// Forward declare for getting NSWindow from GLFW
extern "C" {
    id glfwGetCocoaWindow(GLFWwindow* window);
}

namespace nyanchu {

struct MetalVertex {
    float x, y, z, w;
    float r, g, b, a;
};

class RendererMetalImpl {
public:
    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLBuffer> _vertexBuffer;
    CAMetalLayer* _metalLayer;

    // Per-frame objects
    id<CAMetalDrawable> _drawable;
    id<MTLCommandBuffer> _commandBuffer;
    id<MTLRenderCommandEncoder> _commandEncoder;
    NSAutoreleasePool* _pool;


    RendererMetalImpl(GLFWwindow* window, uint32_t width, uint32_t height) {
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
        _metalLayer.frame = CGRectMake(0, 0, width, height);

        NSWindow* nswin = glfwGetCocoaWindow(window);
        NSView* view = [nswin contentView];
        [view setWantsLayer:YES];
        [view setLayer:_metalLayer];

        const char* shaderSrc = R"(
            using namespace metal;
            struct VertexIn {
                float4 position [[attribute(0)]];
                float4 color [[attribute(1)]];
            };
            struct VertexOut {
                float4 position [[position]];
                float4 color;
            };
            vertex VertexOut vertex_main(const VertexIn in [[stage_in]]) {
                VertexOut out;
                out.position = in.position;
                out.color = in.color;
                return out;
            }
            fragment float4 fragment_main(const VertexOut in [[stage_in]]) {
                return in.color;
            }
        )";

        NSError* error = nil;
        id<MTLLibrary> library = [_device newLibraryWithSource:[NSString stringWithUTF8String:shaderSrc] options:nil error:&error];
        if (!library) {
            std::cerr << "Failed to create Metal library: " << [[error localizedDescription] UTF8String] << std::endl;
            return;
        }

        id<MTLFunction> vertexFunc = [library newFunctionWithName:@"vertex_main"];
        id<MTLFunction> fragmentFunc = [library newFunctionWithName:@"fragment_main"];

        MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDescriptor.vertexFunction = vertexFunc;
        pipelineDescriptor.fragmentFunction = fragmentFunc;
        pipelineDescriptor.colorAttachments[0].pixelFormat = _metalLayer.pixelFormat;

        MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat4;
        vertexDescriptor.attributes[0].offset = 0;
        vertexDescriptor.attributes[0].bufferIndex = 0;
        vertexDescriptor.attributes[1].format = MTLVertexFormatFloat4;
        vertexDescriptor.attributes[1].offset = sizeof(float) * 4;
        vertexDescriptor.attributes[1].bufferIndex = 0;
        vertexDescriptor.layouts[0].stride = sizeof(MetalVertex);

        pipelineDescriptor.vertexDescriptor = vertexDescriptor;

        _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
        if (!_pipelineState) {
            std::cerr << "Failed to create Metal pipeline state: " << [[error localizedDescription] UTF8String] << std::endl;
            return;
        }

        static const MetalVertex vertices[] = {
            { -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
            {  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },
            {  0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f }
        };

        _vertexBuffer = [_device newBufferWithBytes:vertices length:sizeof(vertices) options:MTLResourceStorageModeShared];
    }

    ~RendererMetalImpl() {
        // In a real app, you'd release the Metal objects.
    }

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

        _commandBuffer = [_commandQueue commandBuffer];
        _commandEncoder = [_commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    }

    void endFrame() {
        if (!_commandEncoder) return;

        [_commandEncoder endEncoding];
        [_commandBuffer presentDrawable:_drawable];
        [_commandBuffer commit];

        [_pool drain];
        _pool = nil;
        _drawable = nil;
        _commandBuffer = nil;
        _commandEncoder = nil;
    }

    void drawTriangle() {
        if (!_commandEncoder) return;
        [_commandEncoder setRenderPipelineState:_pipelineState];
        [_commandEncoder setVertexBuffer:_vertexBuffer offset:0 atIndex:0];
        [_commandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    }
};

RendererMetal::RendererMetal() : _impl(nullptr) {}
RendererMetal::~RendererMetal() {
    shutdown();
}

bool RendererMetal::initialize(GLFWwindow* window, uint32_t width, uint32_t height) {
    _impl = new RendererMetalImpl(window, width, height);
    return _impl != nullptr && _impl->_device != nil;
}

void RendererMetal::shutdown() {
    if (_impl) {
        delete _impl;
        _impl = nullptr;
    }
}

void RendererMetal::beginFrame() {
    if (_impl) {
        _impl->beginFrame();
    }
}

void RendererMetal::endFrame() {
    if (_impl) {
        _impl->endFrame();
    }
}

void RendererMetal::drawMesh(const char* meshName) {
    std::cout << "Metal Drawing mesh: " << meshName << std::endl;
}

void RendererMetal::drawTriangle() {
    if (_impl) {
        _impl->drawTriangle();
    }
}

} // namespace nyanchu
