//
// Created by admin on 2025/11/27.
//

#ifndef LYH_PIPELINE_H
#define LYH_PIPELINE_H
#include "descriptor.h"
#include "render_context.h"

namespace lvk {
class Pipeline {
public:
    Pipeline() = default;

    Pipeline(const Pipeline &other) = delete;

    Pipeline &operator=(const Pipeline &) = delete;

    Pipeline &operator=(Pipeline &&other) noexcept {
        descriptorSetLayout = std::move(other.descriptorSetLayout);
        pipelineLayout = other.pipelineLayout;
        graphicsPipeline = other.graphicsPipeline;
        return *this;
    }

    Pipeline(Pipeline &&other) noexcept {
        descriptorSetLayout = std::move(other.descriptorSetLayout);
        pipelineLayout = other.pipelineLayout;
        graphicsPipeline = other.graphicsPipeline;
    }

    class Builder {
    public:
        explicit Builder(const VulkanContext &context) : context(context) {
        }

        Builder &withShader(const std::string &pVertFile, const std::string &pFragFile) {
            vertFile = pVertFile;
            fragFile = pFragFile;
            return *this;
        }

        Builder &withBindingDescription(VkVertexInputBindingDescription description) {
            bindingDescription = description;
            return *this;
        }

        Builder &withVertexDescription(VkVertexInputAttributeDescription description) {
            attributeDescriptions.push_back(description);
            return *this;
        }

        Builder &withVertexDescriptions(std::vector<VkVertexInputAttributeDescription> descriptions) {
            attributeDescriptions = std::move(descriptions);
            return *this;
        }

        Builder &withDescriptorSetLayout(std::unique_ptr<DescriptorSetLayout> &layout) {
            descriptorSetLayout = std::move(layout);
            return *this;
        }

        Pipeline build();

    private:
        const VulkanContext &context;
        std::string vertFile;
        std::string fragFile;

        VkVertexInputBindingDescription bindingDescription{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        VkPipelineLayout pipelineLayout{};
        VkPipeline graphicsPipeline{};

        std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
    };

    friend class Builder;

public:
    std::unique_ptr<DescriptorSetLayout> descriptorSetLayout{};
    // std::unique_ptr<DescriptorPool> descriptorPool;
    // std::vector<VkDescriptorSet> descriptorSets;

    // VulkanContext &context;
    // RenderContext &context;

    // VkRenderPass renderPass{};

    VkPipelineLayout pipelineLayout{};
    VkPipeline graphicsPipeline{};
};
} // end namespace lvk

#endif //LYH_PIPELINE_H
