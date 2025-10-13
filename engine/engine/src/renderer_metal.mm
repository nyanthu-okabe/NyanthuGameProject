#include "nyanchu/renderer_metal.h"
#include "platform/platform_utils.h"
#include <iostream>
#include <unordered_map>

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

struct Uniforms {
    glm::mat4 mvp;
};

struct MeshBuffers {
    id<MTLBuffer> vertexBuffer;
    id<MTLBuffer> indexBuffer;
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

    // Pipeline states
    id<MTLRenderPipelineState> _meshPipelineState;
    id<MTLDepthStencilState> _depthState;

    // Buffers
    id<MTLBuffer> _uniformBuffer;
    std::unordered_map<const Mesh*, MeshBuffers> _meshBuffers;


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

        setupMeshPipeline();
        setupDepthBuffer();

        _uniformBuffer = [_device newBufferWithLength:sizeof(Uniforms) options:MTLResourceStorageModeShared];
    }

    void setupMeshPipeline() {
        const char* shaderSrc = R"(
            using namespace metal;
            struct VertexIn { float3 position [[attribute(0)]]; };
            struct VertexOut { float4 position [[position]]; };
            struct Uniforms { float4x4 mvp; };
            vertex VertexOut vertex_main(const VertexIn in [[stage_in]], constant Uniforms &uniforms [[buffer(1)]]) {
                VertexOut out;
                out.position = uniforms.mvp * float4(in.position, 1.0);
                return out;
            }
            fragment float4 fragment_main() { return float4(0.8, 0.8, 0.8, 1.0); }
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
        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat3; // pos
        vertexDescriptor.attributes[0].offset = offsetof(Vertex, position);
        vertexDescriptor.attributes[0].bufferIndex = 0;
        vertexDescriptor.layouts[0].stride = sizeof(Vertex);
        pipelineDescriptor.vertexDescriptor = vertexDescriptor;

        _meshPipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
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

    void updateUniforms(const glm::mat4& model) {
        if (_width == 0 || _height == 0) return;
        float aspect = (float)_width / (float)_height;
        glm::mat4 proj = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0, 2, -5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        Uniforms uniforms;
        uniforms.mvp = proj * view * model;
        memcpy([_uniformBuffer contents], &uniforms, sizeof(uniforms));
    }

    void drawMesh(const Mesh& mesh, const glm::mat4& modelMatrix) {
        if (!_commandEncoder) return;

        if (_meshBuffers.find(&mesh) == _meshBuffers.end()) {
            const auto& vertices = mesh.getVertices();
            const auto& indices = mesh.getIndices();

            MeshBuffers buffers;
            buffers.vertexBuffer = [_device newBufferWithBytes:vertices.data() length:vertices.size() * sizeof(Vertex) options:MTLResourceStorageModeShared];
            buffers.indexBuffer = [_device newBufferWithBytes:indices.data() length:indices.size() * sizeof(uint32_t) options:MTLResourceStorageModeShared];
            _meshBuffers[&mesh] = buffers;
        }

        const auto& buffers = _meshBuffers.at(&mesh);

        updateUniforms(modelMatrix);

        [_commandEncoder setRenderPipelineState:_meshPipelineState];
        [_commandEncoder setVertexBuffer:buffers.vertexBuffer offset:0 atIndex:0];
        [_commandEncoder setVertexBuffer:_uniformBuffer offset:0 atIndex:1];
        [_commandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                      indexCount:mesh.getIndices().size()
                                       indexType:MTLIndexTypeUInt32
                                     indexBuffer:buffers.indexBuffer
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
void RendererMetal::drawMesh(const Mesh& mesh, const glm::mat4& modelMatrix) { if (_impl) _impl->drawMesh(mesh, modelMatrix); }
void RendererMetal::drawTriangle() { /* Not implemented */ }
void RendererMetal::drawCube(const glm::mat4& modelMatrix) { /* Not implemented */ }

} // namespace nyanchu
