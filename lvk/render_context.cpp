//
// Created by admin on 2025/11/10.
//

#include "render_context.h"
#include <stdexcept>
#include <utility>

namespace lvk {

// RenderContext::RenderContext(VulkanContext &context_): context(context_) {}

RenderContext::RenderContext(VulkanContext &context_, VkRenderPass render_pass_,
                             VkPipelineLayout pipeline_layout_,
                             VkPipeline graphics_pipeline_):
        context(context_), render_pass(render_pass_),
        pipeline_layout(pipeline_layout_), graphics_pipeline(graphics_pipeline_) {

    max_frames_in_flight = 3;

    graphics_queue = context.device.GetQueue(lvk::QueueType::kGraphics);
    present_queue = context.device.GetQueue(lvk::QueueType::kPresent);

    create_framebuffers();
    create_command_pool();
    create_sync_objects();
}

void RenderContext::reset_swapchain(Swapchain swapchain_) {
    // swapchain = swapchain_;
}

void RenderContext::create_framebuffers() {
    swapchain_images = context.swapchain.GetImages();
    swapchain_image_views = context.swapchain.GetImageViews();

    framebuffers.resize(swapchain_image_views.size());

    for (size_t i = 0; i < swapchain_image_views.size(); i++) {
        VkImageView attachments[] = {swapchain_image_views[i]};

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = context.swapchain.extent.width;
        framebuffer_info.height = context.swapchain.extent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(context.device.device, &framebuffer_info, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer");
        }
    }
}

void RenderContext::create_command_pool() {
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = context.device.GetQueueIndex(lvk::QueueType::kGraphics);

    if (vkCreateCommandPool(context.device.device, &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool");
    }
}



void RenderContext::create_sync_objects() {
    available_semaphores.resize(max_frames_in_flight);
    finished_semaphore.resize(context.swapchain.image_count);
    in_flight_fences.resize(max_frames_in_flight);
    image_in_flight.resize(context.swapchain.image_count, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < context.swapchain.image_count; i++) {
        if (vkCreateSemaphore(context.device.device, &semaphore_info, nullptr, &finished_semaphore[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create sync objects");
        }
    }

    for (size_t i = 0; i < max_frames_in_flight; i++) {
        if (vkCreateSemaphore(context.device.device, &semaphore_info, nullptr, &available_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context.device.device, &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create sync objects");
        }
    }
}

void RenderContext::clearup() {

    for (size_t i = 0; i < context.swapchain.image_count; i++) {
        vkDestroySemaphore(context.device.device, finished_semaphore[i], nullptr);
    }
    for (size_t i = 0; i < max_frames_in_flight; i++) {
        vkDestroySemaphore(context.device.device, available_semaphores[i], nullptr);
        vkDestroyFence(context.device.device, in_flight_fences[i], nullptr);
    }

    vkDestroyCommandPool(context.device.device, command_pool, nullptr);

    for (auto framebuffer: framebuffers) {
        vkDestroyFramebuffer(context.device.device, framebuffer, nullptr);
    }

    vkDestroyPipeline(context.device.device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(context.device.device, pipeline_layout, nullptr);
    vkDestroyRenderPass(context.device.device, render_pass, nullptr);

    context.swapchain.DestroyImageViews(swapchain_image_views);

    lvk::destroy_swapchain(context.swapchain);
    lvk::destroy_device(context.device);
    lvk::destroy_surface(context.instance, context.surface);
    lvk::destroy_instance(context.instance);

}

void RenderContext::rendering() {
    vkWaitForFences(context.device.device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index = 0;
    VkResult result = vkAcquireNextImageKHR(context.device.device,
                                            context.swapchain.swapchain, UINT64_MAX, available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain();
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swapchain image. Error " + std::to_string(result));
    }

    if (image_in_flight[image_index] != VK_NULL_HANDLE) {
        vkWaitForFences(context.device.device, 1, &image_in_flight[image_index], VK_TRUE, UINT64_MAX);
    }

    image_in_flight[image_index] = in_flight_fences[current_frame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {available_semaphores[current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = wait_semaphores;
    submitInfo.pWaitDstStageMask = wait_stages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffers[image_index];

    VkSemaphore signal_semaphores[] = {finished_semaphore[image_index]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signal_semaphores;

    vkResetFences(context.device.device, 1, &in_flight_fences[current_frame]);

    if (vkQueueSubmit(graphics_queue, 1, &submitInfo, in_flight_fences[current_frame]) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer");
    }

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapChains[] = {context.swapchain.swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapChains;

    present_info.pImageIndices = &image_index;

    result = vkQueuePresentKHR(present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreate_swapchain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swapchain image\n");
    }

    current_frame = (current_frame + 1) % max_frames_in_flight;
}

void RenderContext::recreate_swapchain() {
    vkDeviceWaitIdle(context.device.device);

    vkDestroyCommandPool(context.device.device, command_pool, nullptr);

    for (auto framebuffer: framebuffers) {
        vkDestroyFramebuffer(context.device.device, framebuffer, nullptr);
    }

    context.swapchain.DestroyImageViews(swapchain_image_views);

    // if (0 != create_swapchain(context)) return ;
    // context.reset_swapchain(init.context.swapchain);
    // create_swapchain();
    context.create_swapchain();
    create_framebuffers();
    create_command_pool();

    // if (0 != create_framebuffers(init, data)) return -1;
    // if (0 != create_command_pool(init, data)) return -1;
    // if (0 != create_command_buffers(init, data)) return -1;
}

} // end namespace lvk