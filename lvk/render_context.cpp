//
// Created by admin on 2025/11/10.
//

#include "render_context.h"

#include <functional>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace lvk {
// RenderContext::RenderContext(VulkanContext &context_): context(context_) {}

RenderContext::RenderContext(VulkanContext &context) : context(context) {
    render_pass = context.GetDefaultRenderPass();
    max_frames_in_flight = 3;

    graphics_queue = context.device.GetQueue(lvk::QueueType::kGraphics);
    present_queue = context.device.GetQueue(lvk::QueueType::kPresent);

    create_framebuffers();
    create_command_pool();
    create_command_buffers();
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
    command_buffers.resize(framebuffers.size());
}

void RenderContext::create_command_pool() {
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // important for rerecord command
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = context.device.GetQueueIndex(lvk::QueueType::kGraphics);

    if (vkCreateCommandPool(context.device.device, &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool");
    }
}

void RenderContext::create_command_buffers() {
    command_buffers.resize(max_frames_in_flight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

    if (vkAllocateCommandBuffers(context.device.device, &allocInfo, command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
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
        if (vkCreateSemaphore(context.device.device, &semaphore_info, nullptr, &available_semaphores[i]) != VK_SUCCESS
            ||
            vkCreateFence(context.device.device, &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create sync objects");
        }
    }
}

void RenderContext::Cleanup() {
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
    // vkDestroyRenderPass(context.device.device, render_pass, nullptr);

    context.swapchain.DestroyImageViews(swapchain_image_views);

    // destroy_swapchain(context.swapchain);
    // destroy_device(context.device);
    // destroy_surface(context.instance, context.surface);
    // destroy_instance(context.instance);
}

void RenderContext::Rendering() {
    vkWaitForFences(context.device.device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index = 0;
    VkResult result = vkAcquireNextImageKHR(context.device.device,
                                            context.swapchain.swapchain, UINT64_MAX,
                                            available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
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
        RecreateSwapchain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swapchain image\n");
    }

    current_frame = (current_frame + 1) % max_frames_in_flight;
}

void RenderContext::Rendering(const std::function<void(RenderContext &)> &draw_record) {
    vkWaitForFences(context.device.device, 1, &in_flight_fences[image_index], VK_TRUE, UINT64_MAX);

    // uint32_t image_index = 0;
    VkResult result = vkAcquireNextImageKHR(context.device.device,
                                            context.swapchain.swapchain, UINT64_MAX,
                                            available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swapchain image. Error " + std::to_string(result));
    }


    if (image_in_flight[image_index] != VK_NULL_HANDLE) {
        vkWaitForFences(context.device.device, 1, &image_in_flight[image_index], VK_TRUE, UINT64_MAX);
    }


    image_in_flight[image_index] = in_flight_fences[current_frame];
    vkResetFences(context.device.device, 1, &in_flight_fences[current_frame]);

    //
    draw_record(*this);

    //

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {available_semaphores[current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = wait_semaphores;
    submitInfo.pWaitDstStageMask = wait_stages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffers[image_index];

    VkSemaphore signal_semaphores[] = {finished_semaphore[current_frame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signal_semaphores;


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
        RecreateSwapchain();
        return;
    }
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swapchain image\n");
    }

    current_frame = (current_frame + 1) % max_frames_in_flight;
}

int RenderContext::RenderBegin() {
    vkWaitForFences(context.device.device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(context.device.device,
                                            context.swapchain.swapchain, UINT64_MAX,
                                            available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        return 1;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swapchain image. Error " + std::to_string(result));
    }

    // vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(command_buffers[image_index], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    if (image_in_flight[image_index] != VK_NULL_HANDLE) {
        vkWaitForFences(context.device.device, 1, &image_in_flight[image_index], VK_TRUE, UINT64_MAX);
    }
    return 0;
}

void RenderContext::RenderEnd() {
    if (vkEndCommandBuffer(command_buffers[image_index]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    image_in_flight[image_index] = in_flight_fences[current_frame];
    vkResetFences(context.device.device, 1, &in_flight_fences[current_frame]);

    //
    VkResult result;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {available_semaphores[current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = wait_semaphores;
    submitInfo.pWaitDstStageMask = wait_stages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffers[image_index];

    VkSemaphore signal_semaphores[] = {finished_semaphore[current_frame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signal_semaphores;


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
        std::cout << "[RenderContext] recreate swapchain width:" << context.swapchain.extent.width << " height:" <<
                context.swapchain.extent.height << std::endl;
        RecreateSwapchain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swapchain image\n");
    }

    current_frame = (current_frame + 1) % max_frames_in_flight;
}

void RenderContext::RenderPassBegin() const {
    auto framebuffer = GetCurrentFrameBuffer();
    // auto commandBuffer = context.command_buffers[current_frame_];
    auto commandBuffer = GetCurrentCommandBuffer();
    auto extent = GetExtent();

    if (debug_mode) {
        std::cout << "[RenderContext] render pass  width:" << extent.width << " height:" << extent.height << std::endl;
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = render_pass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void RenderContext::RenderPassEnd() const {
    auto commandBuffer = GetCurrentCommandBuffer();
    vkCmdEndRenderPass(commandBuffer);
}

void RenderContext::SetDebug(bool is_debug) {
    debug_mode = is_debug;
}

VkCommandBuffer RenderContext::GetCurrentCommandBuffer() const {
    return command_buffers[image_index];
}

uint32_t RenderContext::GetCurrentImageIndex() const {
    return image_index;
}

VkFramebuffer RenderContext::GetCurrentFrameBuffer() const {
    return framebuffers[image_index];
}

VkExtent2D RenderContext::GetExtent() const {
    return context.swapchain.extent;
}


void RenderContext::RecreateSwapchain() {
    vkDeviceWaitIdle(context.device.device);

    vkDestroyCommandPool(context.device.device, command_pool, nullptr);

    for (auto framebuffer: framebuffers) {
        vkDestroyFramebuffer(context.device.device, framebuffer, nullptr);
    }

    context.swapchain.DestroyImageViews(swapchain_image_views);

    // if (0 != create_swapchain(context)) return ;
    // context.reset_swapchain(init.context.swapchain);
    // create_swapchain();
    context.CreateSwapchain();
    create_framebuffers();
    create_command_pool();
    create_command_buffers();

    // if (0 != create_framebuffers(init, data)) return -1;
    // if (0 != create_command_pool(init, data)) return -1;
    // if (0 != create_command_buffers(init, data)) return -1;
}

void RenderContext::ReSize(uint32_t width, uint32_t height) {
    auto extent = GetExtent();
    if (extent.width == width && extent.height == height) {
        return;
    }

    RecreateSwapchain();
    //
    vkDeviceWaitIdle(context.device.device);
}

VkCommandBuffer RenderContext::BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context.device.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void RenderContext::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    // Create fence to ensure that the command buffer has finished executing
    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = 0;

    vkCreateFence(context.device.device, &fence_info, nullptr, &single_fence);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphics_queue, 1, &submitInfo, single_fence);
    // vkQueueWaitIdle(graphics_queue);
    vkWaitForFences(context.device.device, 1, &single_fence, VK_TRUE, 1000000);
    vkDestroyFence(context.device.device, single_fence, nullptr);

    vkFreeCommandBuffers(context.device.device, command_pool, 1, &commandBuffer);
}
} // end namespace lvk
