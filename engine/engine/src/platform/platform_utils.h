#pragma once

#include <GLFW/glfw3.h>
#include <string>

void* getNativeWindowHandle(GLFWwindow* window);
std::string getExecutableDir();
