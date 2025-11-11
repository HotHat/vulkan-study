//
// Created by admin on 2025/11/11.
//

#include "vulkan_context.h"

namespace lvk {

void VulkanContext::create_swapchain() {
    lvk::SwapchainBuilder swapchain_builder{device};
    auto swapchain_ = swapchain_builder.SetOldSwapchain(swapchain).Build();

    lvk::destroy_swapchain(swapchain);

    swapchain = swapchain_;
}

} // end namespace lvk
