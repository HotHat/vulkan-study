//
// Created by admin on 2025/11/21.
//

#ifndef LYH_DRAW_OBJECT_H
#define LYH_DRAW_OBJECT_H
#include <variant>
#include <vector>
#include <glm/vec3.hpp>

#include "image.h"
#include "Texture.h"
#include "Vertex.h"

namespace lvk {
template<typename T>
class DrawObject {
public:
    DrawObject() = default;

    // DrawObject &operator=(DrawObject &&other) noexcept {
    //     vertexes = std::move(other.vertexes);
    //     texture = std::move(other.texture);
    //     indices = other.indices;
    //     pipeline_layout = other.pipeline_layout;
    //     graphics_pipeline = other.graphics_pipeline;
    //
    //     return *this;
    // }

    // DrawObject(DrawObject &&other) noexcept {
    //     vertexes = std::move(other.vertexes);
    //     texture = std::move(other.texture);
    //     indices = other.indices;
    //     pipeline_layout = other.pipeline_layout;
    //     graphics_pipeline = other.graphics_pipeline;
    // }

    // void SetImageView();
    void AddTriangle(T t1, T t2, T t3) {
        uint32_t size = vertexes.size();
        vertexes.emplace_back(t1);
        vertexes.emplace_back(t2);
        vertexes.emplace_back(t3);

        indices.emplace_back(size);
        indices.emplace_back(size + 1);
        indices.emplace_back(size + 2);
    };

    void AddRectangle(const T &t1, const T &t2, const T &t3, const T &t4) {
        uint32_t size = vertexes.size();
        vertexes.emplace_back(t1);
        vertexes.emplace_back(t2);
        vertexes.emplace_back(t3);
        vertexes.emplace_back(t4);

        indices.emplace_back(size);
        indices.emplace_back(size + 1);
        indices.emplace_back(size + 2);
        indices.emplace_back(size + 2);
        indices.emplace_back(size + 3);
        indices.emplace_back(size);
    };

    [[nodiscard]] std::vector<T> GetVertexes() const { return vertexes; };
    [[nodiscard]] std::vector<uint16_t> GetIndices() const { return indices; };
    [[nodiscard]] Texture & GetTexture() const { return *texture; };

    DrawObject &WithPipeline(VkPipeline pipeline) {
        graphics_pipeline = pipeline;
        return *this;
    };

    DrawObject &WithPipelineLayout(VkPipelineLayout layout) {
        pipeline_layout = layout;
        return *this;
    };

    DrawObject &WithTexture(std::unique_ptr<Texture> &p_texture) {
        texture = std::move(p_texture);
        return *this;
    };

    VkPipeline graphics_pipeline{};
    VkPipelineLayout pipeline_layout{};

private:
    std::vector<T> vertexes{};
    std::vector<uint16_t> indices{};


    // VkImageView view = VK_NULL_HANDLE;
    std::unique_ptr<Texture> texture{};
};

using DrawObjectVector2 = DrawObject<Vertex2>;
using DrawObjectVector3 = DrawObject<Vertex3>;

using DrawObjectType = std::variant<DrawObjectVector2, DrawObjectVector3>;

} // end namespace lvk

// #include "draw_object.cpp"

#endif //LYH_DRAW_OBJECT_H
