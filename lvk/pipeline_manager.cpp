//
// Created by admin on 2025/12/1.
//

#include "pipeline_manager.h"

namespace lvk {

ShaderId PipelineManager::Insert(std::unique_ptr<Pipeline> pipeline) {
    pipelines.emplace_back(std::move(pipeline));
    return pipelines.size() - 1;
}

ShaderId PipelineManager::Add(std::string const &name, std::unique_ptr<Pipeline> pipeline) {
    size_t n = Insert(std::move(pipeline));
    nameMap[name] = n;
    return n - 1;
}

} // end namespace lvk
