//
// Created by admin on 2025/11/10.
//

#ifndef LYH_RENDER_CONTEXT_H
#define LYH_RENDER_CONTEXT_H

#include <functional>

#include "vulkan_context.h"
#include <vulkan/vulkan.h>
#include <vector>


namespace lvk {

struct RenderContext {
    // explicit RenderContext(VulkanContext &context_);
    explicit RenderContext(
            VulkanContext &context,
            VkRenderPass render_pass_
            // VkPipelineLayout pipeline_layout_,
            // VkPipeline graphics_pipeline_
    );
    VkQueue graphics_queue{};
    VkQueue present_queue{};

    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    std::vector<VkFramebuffer> framebuffers;

    VkPipelineLayout pipeline_layout{};
    VkPipeline graphics_pipeline{};

    VkRenderPass render_pass;
    VkCommandPool command_pool{};
    std::vector<VkCommandBuffer> command_buffers;

    std::vector<VkSemaphore> available_semaphores;
    std::vector<VkSemaphore> finished_semaphore;
    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> image_in_flight;
    size_t current_frame = 0;
    uint32_t image_index = 0;

    uint8_t max_frames_in_flight = 3;

    VulkanContext &context;

    // Swapchain swapchain;
    // Device device;
    //
    void recreate_swapchain();

    void rendering();
    void rendering(const std::function<void(RenderContext &)>&);
    void RenderBegin();
    void RenderEnd();

    void reset_swapchain(Swapchain swapchain_);
    // void reset_context(VulkanContext context_);
    // void create_swapchain();
    void create_framebuffers();
    void create_command_pool();
    void create_sync_objects();
    void create_command_buffers();

    void clearup();
};

} // end namespace lvk

#endif //LYH_RENDER_CONTEXT_H
