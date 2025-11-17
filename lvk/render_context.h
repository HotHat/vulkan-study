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
class RenderContext {
public:
    explicit RenderContext(
        VulkanContext &context
    );

    void RecreateSwapchain();
    void Rendering();
    void Rendering(const std::function<void(RenderContext &)> &);
    int RenderBegin();
    void RenderEnd();
    void RenderPassBegin() const;
    void RenderPassEnd() const;
    [[nodiscard]] VkCommandBuffer GetCurrentCommandBuffer() const;
    uint32_t GetCurrentImageIndex() const;
    [[nodiscard]] VkFramebuffer GetCurrentFrameBuffer() const;
    [[nodiscard]] VkExtent2D GetExtent() const;
    void Cleanup();

private:
    void reset_swapchain(Swapchain swapchain_);
    // void reset_context(VulkanContext context_);
    // void create_swapchain();
    void create_framebuffers();
    void create_command_pool();
    void create_sync_objects();
    void create_command_buffers();

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
    uint32_t current_frame = 0;
    uint32_t image_index = 0;

    uint8_t max_frames_in_flight = 3;

    VulkanContext &context;

    // Swapchain swapchain;
    // Device device;
};

} // end namespace lvk

#endif //LYH_RENDER_CONTEXT_H
