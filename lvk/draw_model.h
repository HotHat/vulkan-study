//
// Created by admin on 2025/11/14.
//

#ifndef LYH_DRAW_MODEL_H
#define LYH_DRAW_MODEL_H

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "allocator.h"
#include "buffer.h"
#include "render_context.h"

namespace lvk {
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

class DrawModel {
public:
    explicit DrawModel(VulkanContext &context_);

    void load();

    void destroy();

    void draw(RenderContext &context);

    void create_render_pass();
    void create_graphics_pipeline();

    VulkanContext &context;
    std::unique_ptr<Allocator> allocator;
    std::unique_ptr<Buffer> vertice_buffer;
    std::unique_ptr<Buffer> indices_buffer;

    VkRenderPass render_pass {};
    VkPipeline pipeline {};
    VkPipelineLayout pipeline_layout{};
    VkPipeline graphics_pipeline{};

private:
    std::vector<Vertex> vertices{};
    std::vector<uint16_t> indices{};
};
} // end namespace lvk

#endif //LYH_DRAW_MODEL_H
