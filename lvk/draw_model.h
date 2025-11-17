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
#include "descriptor.h"

namespace lvk {
class DescriptorSetLayout;

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

struct GlobalUbo {
    glm::mat4 mvp{1.f};
};

class DrawModel {
public:
    explicit DrawModel(VulkanContext &context);

    void load();
    void load2();
    void load3();

    void destroy();

    void draw(RenderContext &context);

    void create_render_pass();
    void CreateGraphicsPipeline();
    void CreateGraphicsPipeline2();

    VulkanContext &context;
    std::unique_ptr<Allocator> allocator;
    std::unique_ptr<Buffer> vertex_buffer;
    std::unique_ptr<Buffer> indices_buffer;
    std::vector<std::unique_ptr<Buffer>> ubo_buffers;

    VkRenderPass render_pass {};
    VkPipeline pipeline {};
    VkPipelineLayout pipeline_layout{};
    VkPipeline graphics_pipeline{};

private:
    void createDescriptorSet();

    std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
    std::unique_ptr<DescriptorPool> descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<Vertex> vertices{};
    std::vector<uint16_t> indices{};
    GlobalUbo globalUbo{};
};
} // end namespace lvk

#endif //LYH_DRAW_MODEL_H
