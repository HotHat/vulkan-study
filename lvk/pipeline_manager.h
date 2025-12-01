//
// Created by admin on 2025/12/1.
//

#ifndef LYH_PIPELINE_MANAGE_H
#define LYH_PIPELINE_MANAGE_H
#include <vector>

#include "pipeline.h"

namespace lvk {

using ShaderId = size_t;

class PipelineManage {
public:
    static PipelineManage & Instance() {
        static PipelineManage instance;
        return instance;
    }

    PipelineManage(PipelineManage const &) = delete;
    PipelineManage(PipelineManage &&) = delete;
    PipelineManage & operator=(PipelineManage const &) = delete;
    PipelineManage & operator=(PipelineManage &&) = delete;

    ShaderId Insert(std::unique_ptr<Pipeline> pipeline);
    ShaderId Add(std::string const &name, std::unique_ptr<Pipeline> pipeline);

    void Cleanup(VkDevice device) {
        for (auto it = pipelines.begin(); it != pipelines.end(); ++it) {
            (*it)->descriptorSetLayout->Cleanup();
            vkDestroyPipeline(device, (*it)->graphicsPipeline, nullptr);
            vkDestroyPipelineLayout(device, (*it)->pipelineLayout, nullptr);
        }
    }

    [[nodiscard]] const Pipeline &Get(ShaderId shaderId) {
        if (shaderId > pipelines.size()) {
            throw std::runtime_error("Not Shader Id" + std::to_string(shaderId));
        }
        return *pipelines[shaderId];
    }

    [[nodiscard]] ShaderId Get(std::string const & name) const {
        if (!nameMap.contains(name)) {
            throw std::runtime_error("not contain " + name);
        }

        return nameMap.at(name);
    }
private:
    PipelineManage() = default;
    std::vector<std::unique_ptr<Pipeline > > pipelines{};
    std::unordered_map<std::string, size_t> nameMap{};
};


} // end namespace lvk
#endif //LYH_PIPELINE_MANAGE_H
