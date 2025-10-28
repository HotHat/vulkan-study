//
// Created by HotHat on 2025/10/23.
//

#include <iostream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "system_info.h"
#include "instance.h"
#include "physical_device.h"


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

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult glfw_result = glfwCreateWindowSurface(instance.instance, window_, nullptr, &surface);
    if (glfw_result != VK_SUCCESS) {
        std::cerr << "Failed to select create window surface. Error: " << std::to_string(glfw_result) << "\n";
        return EXIT_FAILURE;
    }

    lvk::PhysicalDeviceSelector selector{ instance };
    auto physical_device = selector.SetSurface(surface).Select();

    std::cout << "Success to select physical device: " << physical_device.name << "\n";

    return EXIT_SUCCESS;
}