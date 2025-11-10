//
// Created by admin on 2025/11/10.
//

#include "render_context.h"
#include <stdexcept>
#include <utility>

namespace lvk {

RenderContext::RenderContext(Device device_, Swapchain swapchain_, VkRenderPass render_pass_,
                             VkPipelineLayout pipeline_layout_,
                             VkPipeline graphics_pipeline_):
        device(std::move(device_)), swapchain(swapchain_),
        render_pass(render_pass_), pipeline_layout(pipeline_layout_),
        graphics_pipeline(graphics_pipeline_) {

    max_frames_in_flight = 3;

    graphics_queue = device.GetQueue(lvk::QueueType::kGraphics);
    present_queue = device.GetQueue(lvk::QueueType::kPresent);

    create_framebuffers();
    create_command_pool();
    create_sync_objects();
}

void RenderContext::reset_swapchain(Swapchain swapchain_) {
    swapchain = swapchain_;
}

void RenderContext::create_framebuffers() {
    swapchain_images = swapchain.GetImages();
    swapchain_image_views = swapchain.GetImageViews();

    framebuffers.resize(swapchain_image_views.size());

    for (size_t i = 0; i < swapchain_image_views.size(); i++) {
        VkImageView attachments[] = {swapchain_image_views[i]};

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = swapchain.extent.width;
        framebuffer_info.height = swapchain.extent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(device.device, &framebuffer_info, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer");
        }
    }
}

void RenderContext::create_command_pool() {
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = device.GetQueueIndex(lvk::QueueType::kGraphics);

    if (vkCreateCommandPool(device.device, &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool");
    }
}



void RenderContext::create_sync_objects() {
    available_semaphores.resize(max_frames_in_flight);
    finished_semaphore.resize(swapchain.image_count);
    in_flight_fences.resize(max_frames_in_flight);
    image_in_flight.resize(swapchain.image_count, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < swapchain.image_count; i++) {
        if (vkCreateSemaphore(device.device, &semaphore_info, nullptr, &finished_semaphore[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create sync objects");
        }
    }

    for (size_t i = 0; i < max_frames_in_flight; i++) {
        if (vkCreateSemaphore(device.device, &semaphore_info, nullptr, &available_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device.device, &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create sync objects");
        }
    }
}

void RenderContext::clearup() {

    for (size_t i = 0; i < swapchain.image_count; i++) {
        vkDestroySemaphore(device.device, finished_semaphore[i], nullptr);
    }
    for (size_t i = 0; i < max_frames_in_flight; i++) {
        vkDestroySemaphore(device.device, available_semaphores[i], nullptr);
        vkDestroyFence(device.device, in_flight_fences[i], nullptr);
    }

    vkDestroyCommandPool(device.device, command_pool, nullptr);

    for (auto framebuffer: framebuffers) {
        vkDestroyFramebuffer(device.device, framebuffer, nullptr);
    }

    vkDestroyPipeline(device.device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(device.device, pipeline_layout, nullptr);
    vkDestroyRenderPass(device.device, render_pass, nullptr);

    swapchain.DestroyImageViews(swapchain_image_views);
}

} // end namespace lvk