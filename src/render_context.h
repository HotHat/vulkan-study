//
// Created by admin on 2025/11/10.
//

#ifndef LYH_RENDER_CONTEXT_H
#define LYH_RENDER_CONTEXT_H


#include "swapchain.h"
#include "device.h"
#include <vulkan/vulkan.h>
#include <vector>


namespace lvk {

struct RenderContext {
    explicit RenderContext(
            Device device_,
            Swapchain swapchain_,
            VkRenderPass render_pass_,
            VkPipelineLayout pipeline_layout_,
            VkPipeline graphics_pipeline_
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

    uint8_t max_frames_in_flight;

    Swapchain swapchain;
    Device device;

    void reset_swapchain(Swapchain swapchain_);
    void create_swapchain();
    void create_framebuffers();
    void create_command_pool();
    void create_sync_objects();
    void clearup();
};

} // end namespace lvk

#endif //LYH_RENDER_CONTEXT_H
