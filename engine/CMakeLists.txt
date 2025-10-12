cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

project(nyanthu_engine LANGUAGES CXX OBJCXX)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        0.9.9.8
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
  flecs
  GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
  GIT_TAG        v3.2.5
)
FetchContent_MakeAvailable(flecs)

FetchContent_Declare(
  bgfx
  GIT_REPOSITORY https://github.com/bkaradzic/bgfx.cmake.git
  GIT_TAG master
)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "Build bgfx examples")
set(BGFX_BUILD_TOOLS ON CACHE BOOL "Build bgfx tools")
set(BGFX_INSTALL OFF CACHE BOOL "Enable bgfx installation")
FetchContent_MakeAvailable(bgfx)

find_package(Threads REQUIRED)


set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

# Engine Library
add_library(nyanthu_engine
    engine/src/engine.cpp
    engine/src/ecs_flecs.cpp
    engine/src/audio_miniaudio.cpp
    engine/src/physics_jolt.cpp
    engine/src/mesh.cpp
)

if (APPLE)
    target_sources(nyanthu_engine PRIVATE
        engine/src/platform/platform_utils_macos.mm
        engine/src/renderer_metal.mm
    )
    target_link_libraries(nyanthu_engine PUBLIC
        "-framework Cocoa"
        "-framework Metal"
        "-framework QuartzCore"
        "-framework MetalKit"
    )
elseif(UNIX AND NOT APPLE)
    target_sources(nyanthu_engine PRIVATE
        engine/src/platform/platform_utils_linux.cpp
        engine/src/renderer_opengl.cpp
    )
else()
    target_sources(nyanthu_engine PRIVATE
        engine/src/renderer_opengl.cpp
    )
endif()

target_include_directories(nyanthu_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/include/external
    ${bgfx_SOURCE_DIR}/bgfx/include
    ${bgfx_SOURCE_DIR}/bimg/include
    ${bgfx_SOURCE_DIR}/bx/include
    ${glm_SOURCE_DIR}
    ${flecs_SOURCE_DIR}
    ${glfw_SOURCE_DIR}/include
)

target_link_libraries(nyanthu_engine
    PRIVATE
        bgfx
        bimg
        bx
        glfw
        flecs
        Threads::Threads
        m
)

if(APPLE)
    target_link_libraries(nyanthu_engine PUBLIC
        "-framework Cocoa"
        "-framework Metal"
        "-framework QuartzCore"
    )
endif()

# Example Application (will be handled by application's CMakeLists.txt)
# add_executable(hello_world
#     examples/hello_world/main.cpp
# )
#
# target_link_libraries(hello_world
#     PRIVATE
#         nyanthu_engine
# )
#
# target_compile_definitions(hello_world PRIVATE GLFW_INCLUDE_NONE GLFW_EXPOSE_NATIVE_COCOA)
