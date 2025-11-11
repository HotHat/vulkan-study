//
// Created by admin on 2025/11/11.
//

#ifndef LYH_VULKAN_CONTEXT_H
#define LYH_VULKAN_CONTEXT_H

#include "system_info.h"
#include "instance.h"
#include "device.h"
#include "swapchain.h"

namespace lvk {

struct VulkanContext {

    void create_swapchain();

    Instance instance{};
    VkSurfaceKHR surface{};
    Device device {};
    Swapchain swapchain {};
};


} // end namespace lvk

#endif //LYH_VULKAN_CONTEXT_H
