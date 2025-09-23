#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#include <GLFW/glfw3.h>

// Manually declare glfwGetCocoaWindow with extern "C"
extern "C" {
    id glfwGetCocoaWindow(GLFWwindow* window);
}

#include "platform_utils.h"

#include <mach-o/dyld.h>
#include <limits.h>

// Function to get the Metal layer from a GLFW window
void* getNativeWindowHandle(GLFWwindow* window) {
    NSWindow *nswin = glfwGetCocoaWindow(window);
    if (nswin.contentView.layer == nil) {
        [nswin.contentView setWantsLayer:YES];
        CAMetalLayer* layer = [CAMetalLayer layer];
        nswin.contentView.layer = layer;
    }
    return (__bridge void*)nswin.contentView.layer;
}

std::string getExecutableDir() {
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        std::string path_str(path);
        return path_str.substr(0, path_str.find_last_of("/"));
    } else {
        return ""; // Should not happen
    }
}
