//
// Created by admin on 2025/10/28.
//

#ifndef LVK_FEATURE_CHAIN_H
#define LVK_FEATURE_CHAIN_H
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace lvk {

void compare_feature_struct(VkStructureType sType, std::vector<std::string> & error_list, const void* supported, const void* requested);
void merge_feature_struct(VkStructureType sType, void* current, const void* merge_in);

class FeatureChain {
    struct StructInfo {
        VkStructureType s_type{};
        size_t starting_location{};
        size_t struct_size{};
    };
    std::vector<StructInfo> structure_infos_;
    std::vector<std::byte> structures_;

    std::vector<StructInfo>::const_iterator FindSType(VkStructureType sType) const;

public:
    bool Empty() const;

    bool IsFeatureStructInChain(VkStructureType sType) const;

    // Add a features structure to the FeaturesChain if it isn't present. If it is, merge the already existing structure with structure
    void AddStructure(VkStructureType sType, size_t struct_size, const void* structure);

    // If a structure with sType exists, remove it from the FeatureChain
    void RemoveStructure(VkStructureType sType);

    // Return true if this FeatureChain contains an sType struct and all of the true fields in structure are also true in the FeatureChain struct
    bool Match(VkStructureType sType, const void* structure) const;

    // Add to the error_list all structure fields in requested_features_chain not present in this chain
    void MatchAll(std::vector<std::string>& error_list, FeatureChain const& requested_features_chain) const;

    void CreateChainedFeatures(VkPhysicalDeviceFeatures2& features2);

    std::vector<void*> GetPNextChainMembers();
};


} // end namespace lvk

#endif // LVK_FEATURE_CHAIN_H
