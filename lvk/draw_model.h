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

    void AddDrawObject();

    void AddDrawTextureObject(const std::string &image_path);

    void LoadVertex();

    // void LoadImage();

    void Destroy();

    void DrawTriangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec3 color);
    void DrawRectangle(glm::vec2 pos, glm::vec2 size, glm::vec3 color);
    void DrawRectangleUv(glm::vec2 pos, glm::vec2 size, glm::vec3 color);

    void Draw();

    void UpdateUniform(GlobalUbo &ubo);

    void UpdateUniform2(VkCommandBuffer command_buffer, GlobalUbo &ubo);


    // VulkanContext &context;
    RenderContext &context;


private:
    void createDescriptorSet();

    std::unique_ptr<DescriptorPool> descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    // std::vector<Vertex> vertices{};
    // std::vector<uint16_t> indices{};

    std::vector<std::unique_ptr<BaseDrawObject>> draw_objects{};

    GlobalUbo globalUbo{};


    // std::unique_ptr<Allocator> allocator;
    //
    std::unordered_map<uint32_t, std::unique_ptr<Buffer>> vertex_buffers;
    std::unordered_map<uint32_t, std::unique_ptr<Buffer>> indices_buffers;
    std::unordered_map<uint32_t, std::vector<VkDescriptorSet>> descriptor_sets;
    //
    std::vector<std::unique_ptr<Buffer> > ubo_buffers;
    // std::unique_ptr<Image> texture{};
};

} // end namespace lvk

#endif //LYH_DRAW_MODEL_H
