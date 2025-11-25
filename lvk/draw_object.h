//
// Created by admin on 2025/11/21.
//

#ifndef LYH_DRAW_OBJECT_H
#define LYH_DRAW_OBJECT_H
#include <vector>
#include <glm/vec3.hpp>

#include "image.h"

namespace lvk {
template<typename T>
struct DrawObject {
    // void SetImageView();
    void AddTriangle(T t1, T t2, T t3);
    void AddRectangle(T t1, T t2, T t3, T t4);

    std::vector<T> GetVertexes() { return vertexes; };
    std::vector<T> GetIndices() { return indices; };
    DrawObject& WithPipeline(VkPipeline pipeline) { graphics_pipeline = pipeline; return *this; };
    DrawObject& WithPipelineLayout(VkPipelineLayout layout) { pipeline_layout = layout; return *this; };

    std::vector<T> vertexes{};
    std::vector<uint16_t> indices{};

    VkPipeline graphics_pipeline{};
    VkPipelineLayout pipeline_layout{};

    VkImageView view = VK_NULL_HANDLE;
};
} // end namespace lvk

#include "draw_object.cpp"

#endif //LYH_DRAW_OBJECT_H
