cmake_minimum_required(VERSION 3.16)
project(nyanthu_game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD 20)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

add_compile_options(-Wno-error=incompatible-pointer-types)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=incompatible-pointer-types")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=incompatible-pointer-types")
set(CMAKE_OBJCXX_FLAGS "${CMAKE_OBJCXX_FLAGS} -Wno-error=incompatible-pointer-types")

add_subdirectory(engine)
add_subdirectory(app)
