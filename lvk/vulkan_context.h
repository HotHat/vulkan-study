//
// Created by admin on 2025/11/11.
//

#ifndef LYH_VULKAN_CONTEXT_H
#define LYH_VULKAN_CONTEXT_H

#include <GLFW/glfw3.h>

#include "system_info.h"
#include "instance.h"
#include "device.h"
#include "swapchain.h"

namespace lvk {
struct VulkanContext {
    explicit VulkanContext(GLFWwindow *window, Instance instance, VkSurfaceKHR surface, Device device);
    void CreateSwapchain();
    [[nodiscard]] VkRenderPass GetDefaultRenderPass() const;

    Instance instance{};
    VkSurfaceKHR surface{};
    Device device{};
    Swapchain swapchain{};
    VkRenderPass render_pass{};

    GLFWwindow *window{};

    void Cleanup();

private:
    void createDefaultRenderPass();
};
} // end namespace lvk

#endif //LYH_VULKAN_CONTEXT_H
