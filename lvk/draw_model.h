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
#include "draw_object.h"
#include "Vertex.h"

namespace lvk {

struct GlobalUbo {
    glm::mat4 mvp{1.f};
};

class DrawModel {
public:
    explicit DrawModel(RenderContext &context);

    void load();

    void load2();

    void AddDrawObject();

    void LoadVertex();

    void LoadImage();

    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    void Destroy();

    void DrawTriangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec3 color);
    void DrawRectangle(glm::vec2 pos, glm::vec2 size, glm::vec3 color);

    void Draw();

    void create_render_pass();

    void CreateGraphicsPipeline();

    void CreateGraphicsPipeline2();

    void CreateGraphicsPipeline3(const std::string &vert_file, const std::string &frag_file);

    void UpdateUniform(GlobalUbo &ubo);

    void UpdateUniform2(VkCommandBuffer command_buffer, GlobalUbo &ubo);


    // VulkanContext &context;
    RenderContext &context;


    VkRenderPass render_pass{};
    VkPipeline pipeline{};
    VkPipelineLayout pipeline_layout{};
    VkPipeline graphics_pipeline{};

private:
    void createDescriptorSet();

    std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
    std::unique_ptr<DescriptorPool> descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    // std::vector<Vertex> vertices{};
    // std::vector<uint16_t> indices{};

    std::vector<DrawObject<Vertex2>> draw_objects{};

    GlobalUbo globalUbo{};


    std::unique_ptr<Allocator> allocator;
    std::unordered_map<uint32_t, std::unique_ptr<Buffer>> vertex_buffers;
    std::unordered_map<uint32_t, std::unique_ptr<Buffer>> indices_buffers;
    std::vector<std::unique_ptr<Buffer> > ubo_buffers;
    // std::unique_ptr<Image> texture{};
};

} // end namespace lvk

#endif //LYH_DRAW_MODEL_H
