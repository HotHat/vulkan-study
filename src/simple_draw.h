//
// Created by admin on 2025/11/11.
//

#ifndef LYH_DRAW_CONTEXT_H
#define LYH_DRAW_CONTEXT_H

#include "device.h"
#include "vulkan_context.h"
#include <vulkan/vulkan.h>

namespace lvk {

struct SimpleDraw {

    explicit SimpleDraw(VulkanContext &context_);

    void create_render_pass();
    void create_graphics_pipeline();
    VkShaderModule createShaderModule(const std::vector<char> &code);
    std::vector<char> readFile(const std::string &filename);

    VulkanContext &context;

    VkRenderPass render_pass{};
    VkPipelineLayout pipeline_layout{};
    VkPipeline graphics_pipeline{};

};


} // end namespace lvk

#endif //LYH_DRAW_CONTEXT_H
