//
// Created by admin on 2025/11/21.
//

#ifndef LYH_DRAW_OBJECT_H
#define LYH_DRAW_OBJECT_H
#include <variant>
#include <vector>
#include <glm/vec3.hpp>

#include "image.h"
#include "pipeline.h"
#include "pipeline_manager.h"
#include "Texture.h"
#include "Vertex.h"

namespace lvk {
struct BaseDrawObject {
    explicit BaseDrawObject(ShaderId shaderId) : shaderId(shaderId) {
    };

    BaseDrawObject(const BaseDrawObject &) = delete; // Delete copy constructor
    BaseDrawObject &operator=(const BaseDrawObject &) = delete; // Delete copy assignment

    // Move constructor
    BaseDrawObject(BaseDrawObject &&other) noexcept {
        //
        vertexes = std::move(other.vertexes);
        vertexesSize = other.vertexesSize;
        indices = std::move(other.indices);
        texture = std::move(other.texture);
    }

    // Move assignment
    BaseDrawObject &operator=(BaseDrawObject &&other) {
        //
        vertexes = std::move(other.vertexes);
        vertexesSize = other.vertexesSize;
        indices = std::move(other.indices);
        texture = std::move(other.texture);

        return *this;
    }

    // BaseDrawObject &WithPipeline(Pipeline &pipeline_) {
        // pipeline = std::move(pipeline_);
        // return *this;
    // };

    // BaseDrawObject &WithPipelineLayout(VkPipelineLayout layout) {
    // pipeline_layout = layout;
    // return *this;
    // };

    BaseDrawObject &WithTexture(std::unique_ptr<Texture> &p_texture) {
        texture = std::move(p_texture);
        return *this;
    };

    template<typename T>
    void AddTriangle(const T &t1, const T &t2, const T &t3) {
        auto sz = sizeof(T);
        auto vs = vertexes.size();
        vertexes.resize(vs + sz * 4);
        uint32_t size = vertexesSize;

        std::memcpy(vertexes.data() + vs, &t1, sz);
        std::memcpy(vertexes.data() + vs + sz, &t2, sz);
        std::memcpy(vertexes.data() + vs + sz * 2, &t3, sz);
        vertexesSize += 3;

        indices.emplace_back(size);
        indices.emplace_back(size + 1);
        indices.emplace_back(size + 2);
    };


    template<typename T>
    void AddRectangle(const T &t1, const T &t2, const T &t3, const T &t4) {
        auto sz = sizeof(T);
        auto vs = vertexes.size();
        vertexes.resize(vs + sz * 4);
        uint32_t size = vertexesSize;

        std::memcpy(vertexes.data() + vs, &t1, sz);
        std::memcpy(vertexes.data() + vs + sz, &t2, sz);
        std::memcpy(vertexes.data() + vs + sz * 2, &t3, sz);
        std::memcpy(vertexes.data() + vs + sz * 3, &t4, sz);
        vertexesSize += 4;

        indices.emplace_back(size);
        indices.emplace_back(size + 1);
        indices.emplace_back(size + 2);
        indices.emplace_back(size + 2);
        indices.emplace_back(size + 3);
        indices.emplace_back(size);
    }

    virtual ~BaseDrawObject() = default;


    const void *GetVertexData() const {
        return vertexes.data();
    };

    uint32_t GetVertexDataSize() {
        return vertexes.size();
    };

    const void *GetIndicesData() const {
        assert(!indices.empty() && "indices is empty");
        return indices.data();
    };

    uint32_t GetIndicesDataSize() const {
        assert(!indices.empty() && "indices is empty");
        return indices.size() * sizeof(uint16_t);
    };

    uint32_t GetIndicesSize() const { return indices.size(); };

    bool HasTexture() const { return texture != nullptr; };

    Texture &GetTexture() const { return *texture; };

    VkPipeline GetPipeline() const {
        auto &manage = PipelineManage::Instance();
        auto &pipeline = manage.Get(shaderId);
        return pipeline.graphicsPipeline;
    };

    DescriptorSetLayout &GetDescriptorSetLayout() const {
        auto &pipeline = PipelineManage::Instance().Get(shaderId);
        return *pipeline.descriptorSetLayout;
    };

    VkPipelineLayout GetPipelineLayout() const {
        auto &pipeline = PipelineManage::Instance().Get(shaderId);
        return pipeline.pipelineLayout;
    };

    void Cleanup() const {
        if (texture) {
            texture->Destroy();
        }
    };

protected:
    // VkPipeline graphics_pipeline{};
    // VkPipelineLayout pipeline_layout{};

    //
    // Pipeline pipeline{};
    ShaderId shaderId{};

    //
    std::vector<uint8_t> vertexes{};
    uint32_t vertexesSize = 0;

    std::vector<Vertex2> vertexes2{};
    std::vector<Vertex3> vertexes3{};

    std::vector<uint16_t> indices{};

    // VkImageView view = VK_NULL_HANDLE;
    std::unique_ptr<Texture> texture{};
};

/*
class DrawObjectV2 : public BaseDrawObject {
    const void *GetVertexData() const override {
        assert(!vertexes2.empty() && "vertexes is empty");
        return vertexes2.data();
    };

    uint32_t GetVertexDataSize() const override {
        return vertexes2.size() * sizeof(Vertex2);
    };

};

class DrawObjectV3 : public BaseDrawObject {
    const void *GetVertexData() const override {
        assert(!vertexes3.empty() && "vertexes is empty");
        return vertexes3.data();
    };

    uint32_t GetVertexDataSize() const override {
        return vertexes3.size() * sizeof(Vertex3);
    };
};

template<typename T>
class DrawObject {
public:
    // DrawObject() : BaseDrawObject() {
    // };

    // ~DrawObject() override = default;

    DrawObject &operator=(DrawObject &&other) noexcept {
        vertexes = std::move(other.vertexes);
        texture = std::move(other.texture);
        indices = other.indices;
        pipeline_layout = other.pipeline_layout;
        graphics_pipeline = other.graphics_pipeline;

        return *this;
    }

    DrawObject(DrawObject &&other) noexcept {
        vertexes = std::move(other.vertexes);
        texture = std::move(other.texture);
        indices = other.indices;
        pipeline_layout = other.pipeline_layout;
        graphics_pipeline = other.graphics_pipeline;
    }

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

    [[nodiscard]] const void *GetVertexData() const override {
        assert(!vertexes.empty() && "vertexes is empty");
        return vertexes.data();
    };
    [[nodiscard]] uint32_t GetVertexDataSize() const override { return vertexes.size() * sizeof(T); };

    [[nodiscard]] const void *GetIndices() const {
        assert(!vertexes.empty() && "indices is empty");
        return indices.data();
    };

    [[nodiscard]] uint32_t GetIndicesDataSize() const override { return indices.size() * sizeof(uint16_t); };
    [[nodiscard]] uint32_t GetIndicesSize() const override { return indices.size(); };

    [[nodiscard]] bool HasTexture() const override { return texture != nullptr; };
    [[nodiscard]] Texture &GetTexture() const override { return *texture; };
    [[nodiscard]] VkPipeline GetPipeline() const override { return graphics_pipeline; }

    [[nodiscard]] VkPipelineLayout GetPipelineLayout() const override { return pipeline_layout; }

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

    void Cleanup() const {
        if (texture) {
            texture->Destroy();
        }
    }

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
*/
} // end namespace lvk

// #include "draw_object.cpp"

#endif //LYH_DRAW_OBJECT_H
