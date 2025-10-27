//
// Created by HotHat on 2025/10/23.
//

#include <iostream>
#include <GLFW/glfw3.h>

#include "system_info.h"
#include "instance.h"


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow *window_ = nullptr;
    window_ = glfwCreateWindow(800, 600, "Hello World!", nullptr, nullptr);

    auto info = lvk::SystemInfo::get_system_info();
    auto support = glfwVulkanSupported();
    std::cout << "support: " << support << "\n";

    uint32_t count;
    // 1. Get the number of required instance extensions
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);
    std::cout << "count: " << count << "\n";
    for (int i = 0; i < count; ++i) {
        std::cout << extensions[i] << "\n";
    }

    lvk::InstanceBuilder builder;
    auto instance = builder.SetAppName("Example Vulkan Application")
            .AddAvailableExtensions(count, extensions)
            .RequestValidationLayers ()
            .EnableExtensions(count, extensions)
            .UseDefaultDebugMessenger ()
            .Build ();

    return EXIT_SUCCESS;
}