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

    GLFWwindow *window = nullptr;
    window = glfwCreateWindow(800, 600, "Hello World!", nullptr, nullptr);

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
    VkResult glfw_result = glfwCreateWindowSurface(instance.instance, window, nullptr, &surface);
    if (glfw_result != VK_SUCCESS) {
        std::cerr << "Failed to select create window surface. Error: " << std::to_string(glfw_result) << "\n";
        return EXIT_FAILURE;
    }

    lvk::PhysicalDeviceSelector selector{ instance };
    auto physical_device = selector.SetSurface(surface).Select();

    std::cout << "Success to select physical device: " << physical_device.name << "\n";


    lvk::DeviceBuilder device_builder{ physical_device };
    // automatically propagate needed data from instance & physical device
    auto vkb_device= device_builder.Build();

    std::cout << "Success to select logic device\n";

    // Get the VkDevice handle used in the rest of a vulkan application
    VkDevice device = vkb_device.device;

    // Get the graphics queue with a helper function
    auto graphics_queue = vkb_device.GetQueue(lvk::QueueType::kGraphics);
    auto graphics_queue_index = vkb_device.GetQueueIndex(lvk::QueueType::kGraphics);
    auto present_queue = vkb_device.GetQueue(lvk::QueueType::kPresent);
    auto present_queue_index = vkb_device.GetQueueIndex(lvk::QueueType::kPresent);
    std::cout << "Success to select graphics queue\n";
    std::cout << "Success to select graphics queue index:" << graphics_queue_index << "\n";
    std::cout << "Success to select present queue\n";
    std::cout << "Success to select present queue index:" << present_queue_index << "\n";

    lvk::destroy_device(vkb_device);
    lvk::destroy_surface(instance, surface);
    lvk::destroy_instance(instance);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}