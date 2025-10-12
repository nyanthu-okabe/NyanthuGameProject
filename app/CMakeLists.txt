cmake_minimum_required(VERSION 3.16)

project(game_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

add_executable(game
    src/main.cpp
    src/application.cpp
    src/application.h
)

target_include_directories(game PRIVATE
    ../engine/engine/include
    ../engine/engine/src/platform
    src
)

target_link_libraries(game
    PRIVATE
        nyanthu_engine
)

add_dependencies(game compile_shaders)

if(APPLE)
    target_compile_definitions(game PRIVATE
      GLFW_INCLUDE_NONE
      GLFW_EXPOSE_NATIVE_COCOA
    )
endif()

# Add a custom target to compile shaders
add_custom_target(compile_shaders ALL)

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BUILD_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_BUILD_DIR})

# This path might need to be adjusted depending on where bgfx is built.
# Assuming it's in the main build directory.
set(SHADER_COMPILER ${CMAKE_BINARY_DIR}/_deps/bgfx-build/bin/shaderc)

file(GLOB SHADERS ${SHADER_DIR}/*.sc)

if(SHADERS)
    foreach(SHADER ${SHADERS})
        get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
        set(SHADER_TYPE)
        if(SHADER_NAME MATCHES "^vs_")
            set(SHADER_TYPE "vertex")
        elseif(SHADER_NAME MATCHES "^fs_")
            set(SHADER_TYPE "fragment")
        endif()

        set(OUTPUT_SHADER ${SHADER_BUILD_DIR}/${SHADER_NAME}.bin)

        add_custom_command(
            TARGET compile_shaders
            COMMAND ${SHADER_COMPILER} -f ${SHADER} -o ${OUTPUT_SHADER} --type ${SHADER_TYPE} --platform osx -p metal
            DEPENDS ${SHADER}
            COMMENT "Compiling ${SHADER_NAME}"
        )
    endforeach()
else()
    message(STATUS "No .sc shader files found in ${SHADER_DIR}. Shader compilation will be skipped.")
endif()

# Copy compiled shaders to the build directory
add_custom_command(
    TARGET game
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SHADER_BUILD_DIR}
    $<TARGET_FILE_DIR:game>/shaders
)

add_custom_command(
    TARGET game
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/materials
    $<TARGET_FILE_DIR:game>/materials
)
