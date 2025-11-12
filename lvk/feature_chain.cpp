//
// Created by admin on 2025/10/28.
//

#include "feature_chain.h"
#include <assert.h>

namespace lvk {

bool FeatureChain::Empty() const { return structure_infos_.empty(); }

bool FeatureChain::IsFeatureStructInChain(VkStructureType sType) const {
    return structure_infos_.end() != FindSType(sType);
}

void FeatureChain::AddStructure(VkStructureType sType, size_t struct_size, const void* structure) {
#if !defined(NDEBUG)
    // Validation
    assert(sType != static_cast<VkStructureType>(0) && "Features struct sType must be filled with the struct's "
                                                       "corresponding VkStructureType enum");
    assert(sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 &&
           "Do not pass VkPhysicalDeviceFeatures2 as a required extension feature structure. An "
           "instance of this is managed internally for selection criteria and device creation.");
#endif

    auto found = FindSType(sType);
    if (found != structure_infos_.end()) {
        // Merge structure into the current structure
#if !defined(NDEBUG)
        assert(found->starting_location + found->struct_size <= structures_.size() &&
               "Internal Consistency Error: FeatureChain::add_structure tyring to merge structures into memory that is "
               "past the end of the structures array");
#endif
        merge_feature_struct(sType, &(structures_.at(found->starting_location)), structure);
    } else {
        // Add a structure into the chain
        structure_infos_.push_back(StructInfo{ sType, structures_.size(), struct_size });
        auto& new_structure_info = structure_infos_.back();
        structures_.insert(structures_.end(), struct_size, std::byte(0));
        memcpy(&(structures_.at(new_structure_info.starting_location)), structure, struct_size);
    }
}

void FeatureChain::RemoveStructure(VkStructureType sType) {
    auto found = FindSType(sType);
    if (found != structure_infos_.end()) {
        if (found->starting_location + found->struct_size < structures_.size()) {
            structures_.erase(structures_.begin() + static_cast<int>(found->starting_location),
                             structures_.begin() + static_cast<int>(found->starting_location + found->struct_size));
            structure_infos_.erase(found);
        }
    }
}

bool FeatureChain::Match(VkStructureType sType, const void* structure) const {
    auto found = FindSType(sType);
    if (found != structure_infos_.end()) {
        std::vector<std::string> error_list;
        compare_feature_struct(sType, error_list, &(structures_.at(found->starting_location)), structure);
        return error_list.empty();
    } else {
        return false;
    }
}

void FeatureChain::MatchAll(std::vector<std::string>& error_list, FeatureChain const& requested_features_chain) const {
    if (structure_infos_.size() != requested_features_chain.structure_infos_.size()) {
        return;
    }
    for (size_t i = 0; i < structure_infos_.size(); ++i) {
        compare_feature_struct(structure_infos_.at(i).s_type,
                               error_list,
                               &(structures_.at(structure_infos_.at(i).starting_location)),
                               &(requested_features_chain.structures_.at(requested_features_chain.structure_infos_.at(i).starting_location)));
    }
}

void FeatureChain::CreateChainedFeatures(VkPhysicalDeviceFeatures2& features2) {
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = &structures_.at(0);
    // Write the address of structure N+1 to the pNext member of structure N
    for (size_t i = 0; i < structure_infos_.size() - 1; i++) {
        VkBaseOutStructure structure{};
        memcpy(&structure, &(structures_.at(structure_infos_.at(i).starting_location)), sizeof(VkBaseOutStructure));
        structure.pNext = reinterpret_cast<VkBaseOutStructure*>(&(structures_.at(structure_infos_.at(i + 1).starting_location)));
        memcpy(&(structures_.at(structure_infos_.at(i).starting_location)), &structure, sizeof(VkBaseOutStructure));
    }
    // Write nullptr to the last structures pNext member
    VkBaseOutStructure structure{};
    memcpy(&structure, &(structures_.at(structure_infos_.back().starting_location)), sizeof(VkBaseOutStructure));
    structure.pNext = nullptr;
    memcpy(&(structures_.at(structure_infos_.back().starting_location)), &structure, sizeof(VkBaseOutStructure));
}

std::vector<void*> FeatureChain::GetPNextChainMembers() {
    std::vector<void*> members;
    for (const auto& structure_info : structure_infos_) {
        members.push_back(&(structures_.at(structure_info.starting_location)));
    }
    return members;
}

std::vector<FeatureChain::StructInfo>::const_iterator FeatureChain::FindSType(VkStructureType sType) const {
    return std::find_if(structure_infos_.begin(), structure_infos_.end(), [sType](StructInfo const& struct_info) {
        return struct_info.s_type == sType;
   });
}

void compare_feature_struct(VkStructureType sType, std::vector<std::string> & error_list, const void* supported, const void* requested) {
/*
    switch (sType) {
#if (defined(VK_VERSION_1_1) || defined(VK_KHR_16bit_storage))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES):
            VkPhysicalDevice16BitStorageFeatures(error_list, *reinterpret_cast<const VkPhysicalDevice16BitStorageFeatures*>(supported), *reinterpret_cast<const VkPhysicalDevice16BitStorageFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_1) || defined(VK_KHR_16bit_storage))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR):
            compare_VkPhysicalDevice16BitStorageFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDevice16BitStorageFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDevice16BitStorageFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_1) || defined(VK_KHR_multiview))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES):
            compare_VkPhysicalDeviceMultiviewFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceMultiviewFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceMultiviewFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_1) || defined(VK_KHR_multiview))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHR):
            compare_VkPhysicalDeviceMultiviewFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceMultiviewFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceMultiviewFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_1) || defined(VK_KHR_variable_pointers))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES):
            compare_VkPhysicalDeviceVariablePointersFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceVariablePointersFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceVariablePointersFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_1) || defined(VK_KHR_variable_pointers))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES):
            compare_VkPhysicalDeviceVariablePointerFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceVariablePointerFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceVariablePointerFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_1) || defined(VK_KHR_variable_pointers))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES):
            compare_VkPhysicalDeviceVariablePointerFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceVariablePointerFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceVariablePointerFeaturesKHR*>(requested));
            break;
#elif (defined(VK_VERSION_1_1) || defined(VK_KHR_variable_pointers))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES):
            compare_VkPhysicalDeviceVariablePointersFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceVariablePointersFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceVariablePointersFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_1))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES):
            compare_VkPhysicalDeviceProtectedMemoryFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceProtectedMemoryFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceProtectedMemoryFeatures*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_1) || defined(VK_KHR_sampler_ycbcr_conversion))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES):
            compare_VkPhysicalDeviceSamplerYcbcrConversionFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceSamplerYcbcrConversionFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceSamplerYcbcrConversionFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_1) || defined(VK_KHR_sampler_ycbcr_conversion))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES_KHR):
            compare_VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_1))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES):
            compare_VkPhysicalDeviceShaderDrawParametersFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderDrawParametersFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderDrawParametersFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_1))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES):
            compare_VkPhysicalDeviceShaderDrawParameterFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderDrawParameterFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderDrawParameterFeatures*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES):
            compare_VkPhysicalDeviceVulkan11Features(error_list, *reinterpret_cast<const VkPhysicalDeviceVulkan11Features*>(supported), *reinterpret_cast<const VkPhysicalDeviceVulkan11Features*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES):
            compare_VkPhysicalDeviceVulkan12Features(error_list, *reinterpret_cast<const VkPhysicalDeviceVulkan12Features*>(supported), *reinterpret_cast<const VkPhysicalDeviceVulkan12Features*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2) || defined(VK_KHR_8bit_storage))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES):
            compare_VkPhysicalDevice8BitStorageFeatures(error_list, *reinterpret_cast<const VkPhysicalDevice8BitStorageFeatures*>(supported), *reinterpret_cast<const VkPhysicalDevice8BitStorageFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_2) || defined(VK_KHR_8bit_storage))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR):
            compare_VkPhysicalDevice8BitStorageFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDevice8BitStorageFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDevice8BitStorageFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2) || defined(VK_KHR_shader_atomic_int64))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES):
            compare_VkPhysicalDeviceShaderAtomicInt64Features(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderAtomicInt64Features*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderAtomicInt64Features*>(requested));
            break;
#elif (defined(VK_VERSION_1_2) || defined(VK_KHR_shader_atomic_int64))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderAtomicInt64FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderAtomicInt64FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2) || defined(VK_KHR_shader_float16_int8))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES):
            compare_VkPhysicalDeviceShaderFloat16Int8Features(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderFloat16Int8Features*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderFloat16Int8Features*>(requested));
            break;
#elif (defined(VK_VERSION_1_2) || defined(VK_KHR_shader_float16_int8))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderFloat16Int8FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderFloat16Int8FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderFloat16Int8FeaturesKHR*>(requested));
            break;
#elif (defined(VK_VERSION_1_2) || defined(VK_KHR_shader_float16_int8))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR):
            compare_VkPhysicalDeviceFloat16Int8FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceFloat16Int8FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceFloat16Int8FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2) || defined(VK_EXT_descriptor_indexing))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES):
            compare_VkPhysicalDeviceDescriptorIndexingFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceDescriptorIndexingFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceDescriptorIndexingFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_2) || defined(VK_EXT_descriptor_indexing))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT):
            compare_VkPhysicalDeviceDescriptorIndexingFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceDescriptorIndexingFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceDescriptorIndexingFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2) || defined(VK_EXT_scalar_block_layout))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES):
            compare_VkPhysicalDeviceScalarBlockLayoutFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceScalarBlockLayoutFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceScalarBlockLayoutFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_2) || defined(VK_EXT_scalar_block_layout))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES_EXT):
            compare_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceScalarBlockLayoutFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceScalarBlockLayoutFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2) || defined(VK_KHR_vulkan_memory_model))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES):
            compare_VkPhysicalDeviceVulkanMemoryModelFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceVulkanMemoryModelFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceVulkanMemoryModelFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_2) || defined(VK_KHR_vulkan_memory_model))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES_KHR):
            compare_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceVulkanMemoryModelFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceVulkanMemoryModelFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2) || defined(VK_KHR_imageless_framebuffer))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES):
            compare_VkPhysicalDeviceImagelessFramebufferFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceImagelessFramebufferFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceImagelessFramebufferFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_2) || defined(VK_KHR_imageless_framebuffer))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES_KHR):
            compare_VkPhysicalDeviceImagelessFramebufferFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceImagelessFramebufferFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceImagelessFramebufferFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2) || defined(VK_KHR_uniform_buffer_standard_layout))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES):
            compare_VkPhysicalDeviceUniformBufferStandardLayoutFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceUniformBufferStandardLayoutFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceUniformBufferStandardLayoutFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_2) || defined(VK_KHR_uniform_buffer_standard_layout))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES_KHR):
            compare_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2) || defined(VK_KHR_shader_subgroup_extended_types))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES):
            compare_VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_2) || defined(VK_KHR_shader_subgroup_extended_types))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2) || defined(VK_KHR_separate_depth_stencil_layouts))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES):
            compare_VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_2) || defined(VK_KHR_separate_depth_stencil_layouts))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES_KHR):
            compare_VkPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2) || defined(VK_EXT_host_query_reset))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES):
            compare_VkPhysicalDeviceHostQueryResetFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceHostQueryResetFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceHostQueryResetFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_2) || defined(VK_EXT_host_query_reset))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT):
            compare_VkPhysicalDeviceHostQueryResetFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceHostQueryResetFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceHostQueryResetFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2) || defined(VK_KHR_timeline_semaphore))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES):
            compare_VkPhysicalDeviceTimelineSemaphoreFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_2) || defined(VK_KHR_timeline_semaphore))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR):
            compare_VkPhysicalDeviceTimelineSemaphoreFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_2) || defined(VK_KHR_buffer_device_address))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES):
            compare_VkPhysicalDeviceBufferDeviceAddressFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_2) || defined(VK_KHR_buffer_device_address))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR):
            compare_VkPhysicalDeviceBufferDeviceAddressFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_3))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES):
            compare_VkPhysicalDeviceVulkan13Features(error_list, *reinterpret_cast<const VkPhysicalDeviceVulkan13Features*>(supported), *reinterpret_cast<const VkPhysicalDeviceVulkan13Features*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_3) || defined(VK_KHR_shader_terminate_invocation))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES):
            compare_VkPhysicalDeviceShaderTerminateInvocationFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderTerminateInvocationFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderTerminateInvocationFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_3) || defined(VK_KHR_shader_terminate_invocation))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_3) || defined(VK_EXT_shader_demote_to_helper_invocation))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES):
            compare_VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_3) || defined(VK_EXT_shader_demote_to_helper_invocation))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT):
            compare_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_3) || defined(VK_EXT_private_data))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES):
            compare_VkPhysicalDevicePrivateDataFeatures(error_list, *reinterpret_cast<const VkPhysicalDevicePrivateDataFeatures*>(supported), *reinterpret_cast<const VkPhysicalDevicePrivateDataFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_3) || defined(VK_EXT_private_data))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES_EXT):
            compare_VkPhysicalDevicePrivateDataFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDevicePrivateDataFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDevicePrivateDataFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_3) || defined(VK_EXT_pipeline_creation_cache_control))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES):
            compare_VkPhysicalDevicePipelineCreationCacheControlFeatures(error_list, *reinterpret_cast<const VkPhysicalDevicePipelineCreationCacheControlFeatures*>(supported), *reinterpret_cast<const VkPhysicalDevicePipelineCreationCacheControlFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_3) || defined(VK_EXT_pipeline_creation_cache_control))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES_EXT):
            compare_VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_3) || defined(VK_KHR_synchronization2))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES):
            compare_VkPhysicalDeviceSynchronization2Features(error_list, *reinterpret_cast<const VkPhysicalDeviceSynchronization2Features*>(supported), *reinterpret_cast<const VkPhysicalDeviceSynchronization2Features*>(requested));
            break;
#elif (defined(VK_VERSION_1_3) || defined(VK_KHR_synchronization2))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR):
            compare_VkPhysicalDeviceSynchronization2FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceSynchronization2FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceSynchronization2FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_3) || defined(VK_KHR_zero_initialize_workgroup_memory))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES):
            compare_VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_3) || defined(VK_KHR_zero_initialize_workgroup_memory))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES_KHR):
            compare_VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_3) || defined(VK_EXT_image_robustness))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES):
            compare_VkPhysicalDeviceImageRobustnessFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceImageRobustnessFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceImageRobustnessFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_3) || defined(VK_EXT_image_robustness))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES_EXT):
            compare_VkPhysicalDeviceImageRobustnessFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceImageRobustnessFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceImageRobustnessFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_3) || defined(VK_EXT_subgroup_size_control))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES):
            compare_VkPhysicalDeviceSubgroupSizeControlFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceSubgroupSizeControlFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceSubgroupSizeControlFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_3) || defined(VK_EXT_subgroup_size_control))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT):
            compare_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceSubgroupSizeControlFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceSubgroupSizeControlFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_3) || defined(VK_EXT_inline_uniform_block))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES):
            compare_VkPhysicalDeviceInlineUniformBlockFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceInlineUniformBlockFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceInlineUniformBlockFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_3) || defined(VK_EXT_inline_uniform_block))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT):
            compare_VkPhysicalDeviceInlineUniformBlockFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceInlineUniformBlockFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceInlineUniformBlockFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_3) || defined(VK_EXT_texture_compression_astc_hdr))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES):
            compare_VkPhysicalDeviceTextureCompressionASTCHDRFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceTextureCompressionASTCHDRFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceTextureCompressionASTCHDRFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_3) || defined(VK_EXT_texture_compression_astc_hdr))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES_EXT):
            compare_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_3) || defined(VK_KHR_dynamic_rendering))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES):
            compare_VkPhysicalDeviceDynamicRenderingFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceDynamicRenderingFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceDynamicRenderingFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_3) || defined(VK_KHR_dynamic_rendering))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR):
            compare_VkPhysicalDeviceDynamicRenderingFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceDynamicRenderingFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceDynamicRenderingFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_3) || defined(VK_KHR_shader_integer_dot_product))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES):
            compare_VkPhysicalDeviceShaderIntegerDotProductFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderIntegerDotProductFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderIntegerDotProductFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_3) || defined(VK_KHR_shader_integer_dot_product))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_3) || defined(VK_KHR_maintenance4))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES):
            compare_VkPhysicalDeviceMaintenance4Features(error_list, *reinterpret_cast<const VkPhysicalDeviceMaintenance4Features*>(supported), *reinterpret_cast<const VkPhysicalDeviceMaintenance4Features*>(requested));
            break;
#elif (defined(VK_VERSION_1_3) || defined(VK_KHR_maintenance4))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES_KHR):
            compare_VkPhysicalDeviceMaintenance4FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceMaintenance4FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceMaintenance4FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_4))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES):
            compare_VkPhysicalDeviceVulkan14Features(error_list, *reinterpret_cast<const VkPhysicalDeviceVulkan14Features*>(supported), *reinterpret_cast<const VkPhysicalDeviceVulkan14Features*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_4) || defined(VK_KHR_global_priority) || defined(VK_EXT_global_priority_query))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES):
            compare_VkPhysicalDeviceGlobalPriorityQueryFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceGlobalPriorityQueryFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceGlobalPriorityQueryFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_KHR_global_priority))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES_KHR):
            compare_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_EXT_global_priority_query))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES_KHR):
            compare_VkPhysicalDeviceGlobalPriorityQueryFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceGlobalPriorityQueryFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceGlobalPriorityQueryFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_4) || defined(VK_KHR_shader_subgroup_rotate))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES):
            compare_VkPhysicalDeviceShaderSubgroupRotateFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderSubgroupRotateFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderSubgroupRotateFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_KHR_shader_subgroup_rotate))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderSubgroupRotateFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderSubgroupRotateFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderSubgroupRotateFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_4) || defined(VK_KHR_shader_float_controls2))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES):
            compare_VkPhysicalDeviceShaderFloatControls2Features(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderFloatControls2Features*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderFloatControls2Features*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_KHR_shader_float_controls2))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderFloatControls2FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderFloatControls2FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderFloatControls2FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_4) || defined(VK_KHR_shader_expect_assume))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES):
            compare_VkPhysicalDeviceShaderExpectAssumeFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderExpectAssumeFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderExpectAssumeFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_KHR_shader_expect_assume))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderExpectAssumeFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderExpectAssumeFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderExpectAssumeFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_4) || defined(VK_KHR_line_rasterization) || defined(VK_EXT_line_rasterization))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES):
            compare_VkPhysicalDeviceLineRasterizationFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceLineRasterizationFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceLineRasterizationFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_KHR_line_rasterization))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT):
            compare_VkPhysicalDeviceLineRasterizationFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceLineRasterizationFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceLineRasterizationFeaturesKHR*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_EXT_line_rasterization))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT):
            compare_VkPhysicalDeviceLineRasterizationFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceLineRasterizationFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceLineRasterizationFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_4) || defined(VK_KHR_vertex_attribute_divisor) || defined(VK_EXT_vertex_attribute_divisor))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES):
            compare_VkPhysicalDeviceVertexAttributeDivisorFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_KHR_vertex_attribute_divisor))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT):
            compare_VkPhysicalDeviceVertexAttributeDivisorFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorFeaturesKHR*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_EXT_vertex_attribute_divisor))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT):
            compare_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_4) || defined(VK_KHR_index_type_uint8) || defined(VK_EXT_index_type_uint8))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES):
            compare_VkPhysicalDeviceIndexTypeUint8Features(error_list, *reinterpret_cast<const VkPhysicalDeviceIndexTypeUint8Features*>(supported), *reinterpret_cast<const VkPhysicalDeviceIndexTypeUint8Features*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_KHR_index_type_uint8))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT):
            compare_VkPhysicalDeviceIndexTypeUint8FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceIndexTypeUint8FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceIndexTypeUint8FeaturesKHR*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_EXT_index_type_uint8))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT):
            compare_VkPhysicalDeviceIndexTypeUint8FeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceIndexTypeUint8FeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceIndexTypeUint8FeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_4) || defined(VK_KHR_maintenance5))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES):
            compare_VkPhysicalDeviceMaintenance5Features(error_list, *reinterpret_cast<const VkPhysicalDeviceMaintenance5Features*>(supported), *reinterpret_cast<const VkPhysicalDeviceMaintenance5Features*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_KHR_maintenance5))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR):
            compare_VkPhysicalDeviceMaintenance5FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceMaintenance5FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceMaintenance5FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_4) || defined(VK_KHR_dynamic_rendering_local_read))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES):
            compare_VkPhysicalDeviceDynamicRenderingLocalReadFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceDynamicRenderingLocalReadFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceDynamicRenderingLocalReadFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_KHR_dynamic_rendering_local_read))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR):
            compare_VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_4) || defined(VK_KHR_maintenance6))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES):
            compare_VkPhysicalDeviceMaintenance6Features(error_list, *reinterpret_cast<const VkPhysicalDeviceMaintenance6Features*>(supported), *reinterpret_cast<const VkPhysicalDeviceMaintenance6Features*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_KHR_maintenance6))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES_KHR):
            compare_VkPhysicalDeviceMaintenance6FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceMaintenance6FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceMaintenance6FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_4) || defined(VK_EXT_pipeline_protected_access))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES):
            compare_VkPhysicalDevicePipelineProtectedAccessFeatures(error_list, *reinterpret_cast<const VkPhysicalDevicePipelineProtectedAccessFeatures*>(supported), *reinterpret_cast<const VkPhysicalDevicePipelineProtectedAccessFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_EXT_pipeline_protected_access))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES_EXT):
            compare_VkPhysicalDevicePipelineProtectedAccessFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDevicePipelineProtectedAccessFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDevicePipelineProtectedAccessFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_4) || defined(VK_EXT_pipeline_robustness))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES):
            compare_VkPhysicalDevicePipelineRobustnessFeatures(error_list, *reinterpret_cast<const VkPhysicalDevicePipelineRobustnessFeatures*>(supported), *reinterpret_cast<const VkPhysicalDevicePipelineRobustnessFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_EXT_pipeline_robustness))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES_EXT):
            compare_VkPhysicalDevicePipelineRobustnessFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDevicePipelineRobustnessFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDevicePipelineRobustnessFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VERSION_1_4) || defined(VK_EXT_host_image_copy))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES):
            compare_VkPhysicalDeviceHostImageCopyFeatures(error_list, *reinterpret_cast<const VkPhysicalDeviceHostImageCopyFeatures*>(supported), *reinterpret_cast<const VkPhysicalDeviceHostImageCopyFeatures*>(requested));
            break;
#elif (defined(VK_VERSION_1_4) || defined(VK_EXT_host_image_copy))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT):
            compare_VkPhysicalDeviceHostImageCopyFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceHostImageCopyFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceHostImageCopyFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_KHR_performance_query))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR):
            compare_VkPhysicalDevicePerformanceQueryFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDevicePerformanceQueryFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDevicePerformanceQueryFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_shader_bfloat16))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_BFLOAT16_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderBfloat16FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderBfloat16FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderBfloat16FeaturesKHR*>(requested));
            break;
#endif
#if defined(VK_ENABLE_BETA_EXTENSIONS) && (defined(VK_KHR_portability_subset))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR):
            compare_VkPhysicalDevicePortabilitySubsetFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDevicePortabilitySubsetFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDevicePortabilitySubsetFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_shader_clock))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderClockFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderClockFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderClockFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_fragment_shading_rate))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR):
            compare_VkPhysicalDeviceFragmentShadingRateFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_shader_quad_control))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderQuadControlFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderQuadControlFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderQuadControlFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_present_wait))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR):
            compare_VkPhysicalDevicePresentWaitFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDevicePresentWaitFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDevicePresentWaitFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_pipeline_executable_properties))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR):
            compare_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_present_id))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR):
            compare_VkPhysicalDevicePresentIdFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDevicePresentIdFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDevicePresentIdFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_fragment_shader_barycentric))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR):
            compare_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR*>(requested));
            break;
#elif (defined(VK_NV_fragment_shader_barycentric))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_NV):
            compare_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_KHR_shader_subgroup_uniform_control_flow))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_workgroup_memory_explicit_layout))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR):
            compare_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_ray_tracing_maintenance1))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR):
            compare_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_shader_untyped_pointers))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_UNTYPED_POINTERS_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderUntypedPointersFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderUntypedPointersFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderUntypedPointersFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_shader_maximal_reconvergence))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_present_id2))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_2_FEATURES_KHR):
            compare_VkPhysicalDevicePresentId2FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDevicePresentId2FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDevicePresentId2FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_present_wait2))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_2_FEATURES_KHR):
            compare_VkPhysicalDevicePresentWait2FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDevicePresentWait2FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDevicePresentWait2FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_ray_tracing_position_fetch))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR):
            compare_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_pipeline_binary))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_FEATURES_KHR):
            compare_VkPhysicalDevicePipelineBinaryFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDevicePipelineBinaryFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDevicePipelineBinaryFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_swapchain_maintenance1))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR):
            compare_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR*>(requested));
            break;
#elif (defined(VK_EXT_swapchain_maintenance1))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT):
            compare_VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_KHR_cooperative_matrix))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR):
            compare_VkPhysicalDeviceCooperativeMatrixFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_compute_shader_derivatives))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR):
            compare_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR*>(requested));
            break;
#elif (defined(VK_NV_compute_shader_derivatives))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_NV):
            compare_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceComputeShaderDerivativesFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceComputeShaderDerivativesFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_KHR_video_encode_av1))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_AV1_FEATURES_KHR):
            compare_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceVideoEncodeAV1FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceVideoEncodeAV1FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_video_decode_vp9))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_DECODE_VP9_FEATURES_KHR):
            compare_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceVideoDecodeVP9FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceVideoDecodeVP9FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_video_maintenance1))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_1_FEATURES_KHR):
            compare_VkPhysicalDeviceVideoMaintenance1FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceVideoMaintenance1FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceVideoMaintenance1FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_unified_image_layouts))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR):
            compare_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_copy_memory_indirect))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_KHR):
            compare_VkPhysicalDeviceCopyMemoryIndirectFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceCopyMemoryIndirectFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceCopyMemoryIndirectFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_video_encode_intra_refresh))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_INTRA_REFRESH_FEATURES_KHR):
            compare_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_video_encode_quantization_map))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUANTIZATION_MAP_FEATURES_KHR):
            compare_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_shader_relaxed_extended_instruction))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_maintenance7))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR):
            compare_VkPhysicalDeviceMaintenance7FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceMaintenance7FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceMaintenance7FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_maintenance8))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR):
            compare_VkPhysicalDeviceMaintenance8FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceMaintenance8FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceMaintenance8FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_shader_fma))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FMA_FEATURES_KHR):
            compare_VkPhysicalDeviceShaderFmaFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderFmaFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderFmaFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_maintenance9))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_FEATURES_KHR):
            compare_VkPhysicalDeviceMaintenance9FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceMaintenance9FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceMaintenance9FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_video_maintenance2))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_2_FEATURES_KHR):
            compare_VkPhysicalDeviceVideoMaintenance2FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceVideoMaintenance2FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceVideoMaintenance2FeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_depth_clamp_zero_one))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_KHR):
            compare_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceDepthClampZeroOneFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceDepthClampZeroOneFeaturesKHR*>(requested));
            break;
#elif (defined(VK_EXT_depth_clamp_zero_one))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_EXT):
            compare_VkPhysicalDeviceDepthClampZeroOneFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceDepthClampZeroOneFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceDepthClampZeroOneFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_KHR_robustness2))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_KHR):
            compare_VkPhysicalDeviceRobustness2FeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceRobustness2FeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceRobustness2FeaturesKHR*>(requested));
            break;
#elif (defined(VK_EXT_robustness2))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT):
            compare_VkPhysicalDeviceRobustness2FeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceRobustness2FeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceRobustness2FeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_KHR_present_mode_fifo_latest_ready))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_KHR):
            compare_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR*>(requested));
            break;
#elif (defined(VK_EXT_present_mode_fifo_latest_ready))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_EXT):
            compare_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_transform_feedback))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT):
            compare_VkPhysicalDeviceTransformFeedbackFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceTransformFeedbackFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceTransformFeedbackFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_NV_corner_sampled_image))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV):
            compare_VkPhysicalDeviceCornerSampledImageFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceCornerSampledImageFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceCornerSampledImageFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_EXT_astc_decode_mode))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT):
            compare_VkPhysicalDeviceASTCDecodeFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceASTCDecodeFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceASTCDecodeFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_conditional_rendering))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT):
            compare_VkPhysicalDeviceConditionalRenderingFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceConditionalRenderingFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceConditionalRenderingFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_depth_clip_enable))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT):
            compare_VkPhysicalDeviceDepthClipEnableFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceDepthClipEnableFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceDepthClipEnableFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_IMG_relaxed_line_rasterization))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RELAXED_LINE_RASTERIZATION_FEATURES_IMG):
            compare_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG(error_list, *reinterpret_cast<const VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG*>(supported), *reinterpret_cast<const VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG*>(requested));
            break;
#endif
#if defined(VK_ENABLE_BETA_EXTENSIONS) && (defined(VK_AMDX_shader_enqueue))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_FEATURES_AMDX):
            compare_VkPhysicalDeviceShaderEnqueueFeaturesAMDX(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderEnqueueFeaturesAMDX*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderEnqueueFeaturesAMDX*>(requested));
            break;
#endif
#if (defined(VK_EXT_blend_operation_advanced))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT):
            compare_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_NV_shader_sm_builtins))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV):
            compare_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_shading_rate_image))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV):
            compare_VkPhysicalDeviceShadingRateImageFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceShadingRateImageFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceShadingRateImageFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_representative_fragment_test))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV):
            compare_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_mesh_shader))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV):
            compare_VkPhysicalDeviceMeshShaderFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_shader_image_footprint))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV):
            compare_VkPhysicalDeviceShaderImageFootprintFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderImageFootprintFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderImageFootprintFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_scissor_exclusive))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV):
            compare_VkPhysicalDeviceExclusiveScissorFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceExclusiveScissorFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceExclusiveScissorFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_INTEL_shader_integer_functions2))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL):
            compare_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL*>(requested));
            break;
#endif
#if (defined(VK_EXT_fragment_density_map))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT):
            compare_VkPhysicalDeviceFragmentDensityMapFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_AMD_device_coherent_memory))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD):
            compare_VkPhysicalDeviceCoherentMemoryFeaturesAMD(error_list, *reinterpret_cast<const VkPhysicalDeviceCoherentMemoryFeaturesAMD*>(supported), *reinterpret_cast<const VkPhysicalDeviceCoherentMemoryFeaturesAMD*>(requested));
            break;
#endif
#if (defined(VK_EXT_shader_image_atomic_int64))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT):
            compare_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_memory_priority))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT):
            compare_VkPhysicalDeviceMemoryPriorityFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceMemoryPriorityFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceMemoryPriorityFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_NV_dedicated_allocation_image_aliasing))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV):
            compare_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_EXT_buffer_device_address))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT):
            compare_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT*>(requested));
            break;
#elif (defined(VK_EXT_buffer_device_address))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_ADDRESS_FEATURES_EXT):
            compare_VkPhysicalDeviceBufferAddressFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceBufferAddressFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceBufferAddressFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_NV_cooperative_matrix))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV):
            compare_VkPhysicalDeviceCooperativeMatrixFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_coverage_reduction_mode))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV):
            compare_VkPhysicalDeviceCoverageReductionModeFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceCoverageReductionModeFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceCoverageReductionModeFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_EXT_fragment_shader_interlock))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT):
            compare_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_ycbcr_image_arrays))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT):
            compare_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_provoking_vertex))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT):
            compare_VkPhysicalDeviceProvokingVertexFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceProvokingVertexFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceProvokingVertexFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_shader_atomic_float))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT):
            compare_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_extended_dynamic_state))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT):
            compare_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_map_memory_placed))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_FEATURES_EXT):
            compare_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceMapMemoryPlacedFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceMapMemoryPlacedFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_shader_atomic_float2))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT):
            compare_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_NV_device_generated_commands))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV):
            compare_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_inherited_viewport_scissor))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INHERITED_VIEWPORT_SCISSOR_FEATURES_NV):
            compare_VkPhysicalDeviceInheritedViewportScissorFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceInheritedViewportScissorFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceInheritedViewportScissorFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_EXT_texel_buffer_alignment))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT):
            compare_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_depth_bias_control))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT):
            compare_VkPhysicalDeviceDepthBiasControlFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceDepthBiasControlFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceDepthBiasControlFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_device_memory_report))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT):
            compare_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_custom_border_color))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT):
            compare_VkPhysicalDeviceCustomBorderColorFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceCustomBorderColorFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceCustomBorderColorFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_NV_present_barrier))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_BARRIER_FEATURES_NV):
            compare_VkPhysicalDevicePresentBarrierFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDevicePresentBarrierFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDevicePresentBarrierFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_device_diagnostics_config))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV):
            compare_VkPhysicalDeviceDiagnosticsConfigFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceDiagnosticsConfigFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceDiagnosticsConfigFeaturesNV*>(requested));
            break;
#endif
#if defined(VK_ENABLE_BETA_EXTENSIONS) && (defined(VK_NV_cuda_kernel_launch))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_FEATURES_NV):
            compare_VkPhysicalDeviceCudaKernelLaunchFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceCudaKernelLaunchFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceCudaKernelLaunchFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_QCOM_tile_shading))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_SHADING_FEATURES_QCOM):
            compare_VkPhysicalDeviceTileShadingFeaturesQCOM(error_list, *reinterpret_cast<const VkPhysicalDeviceTileShadingFeaturesQCOM*>(supported), *reinterpret_cast<const VkPhysicalDeviceTileShadingFeaturesQCOM*>(requested));
            break;
#endif
#if (defined(VK_EXT_descriptor_buffer))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT):
            compare_VkPhysicalDeviceDescriptorBufferFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceDescriptorBufferFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceDescriptorBufferFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_graphics_pipeline_library))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT):
            compare_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_AMD_shader_early_and_late_fragment_tests))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_FEATURES_AMD):
            compare_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD*>(requested));
            break;
#endif
#if (defined(VK_NV_fragment_shading_rate_enums))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_FEATURES_NV):
            compare_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_ray_tracing_motion_blur))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MOTION_BLUR_FEATURES_NV):
            compare_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceRayTracingMotionBlurFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceRayTracingMotionBlurFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_EXT_ycbcr_2plane_444_formats))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT):
            compare_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_fragment_density_map2))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT):
            compare_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_image_compression_control))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT):
            compare_VkPhysicalDeviceImageCompressionControlFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceImageCompressionControlFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceImageCompressionControlFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_attachment_feedback_loop_layout))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT):
            compare_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_4444_formats))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT):
            compare_VkPhysicalDevice4444FormatsFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDevice4444FormatsFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDevice4444FormatsFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_device_fault))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT):
            compare_VkPhysicalDeviceFaultFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceFaultFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceFaultFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_rasterization_order_attachment_access))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT):
            compare_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT*>(requested));
            break;
#elif (defined(VK_ARM_rasterization_order_attachment_access))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_ARM):
            compare_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM(error_list, *reinterpret_cast<const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM*>(supported), *reinterpret_cast<const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM*>(requested));
            break;
#endif
#if (defined(VK_EXT_rgba10x6_formats))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT):
            compare_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_mutable_descriptor_type))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT):
            compare_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT*>(requested));
            break;
#elif (defined(VK_VALVE_mutable_descriptor_type))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_VALVE):
            compare_VkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE(error_list, *reinterpret_cast<const VkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE*>(supported), *reinterpret_cast<const VkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE*>(requested));
            break;
#endif
#if (defined(VK_EXT_vertex_input_dynamic_state))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT):
            compare_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_device_address_binding_report))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT):
            compare_VkPhysicalDeviceAddressBindingReportFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceAddressBindingReportFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceAddressBindingReportFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_depth_clip_control))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT):
            compare_VkPhysicalDeviceDepthClipControlFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceDepthClipControlFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceDepthClipControlFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_primitive_topology_list_restart))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT):
            compare_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_HUAWEI_subpass_shading))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_FEATURES_HUAWEI):
            compare_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI(error_list, *reinterpret_cast<const VkPhysicalDeviceSubpassShadingFeaturesHUAWEI*>(supported), *reinterpret_cast<const VkPhysicalDeviceSubpassShadingFeaturesHUAWEI*>(requested));
            break;
#endif
#if (defined(VK_HUAWEI_invocation_mask))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INVOCATION_MASK_FEATURES_HUAWEI):
            compare_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI(error_list, *reinterpret_cast<const VkPhysicalDeviceInvocationMaskFeaturesHUAWEI*>(supported), *reinterpret_cast<const VkPhysicalDeviceInvocationMaskFeaturesHUAWEI*>(requested));
            break;
#endif
#if (defined(VK_NV_external_memory_rdma))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_RDMA_FEATURES_NV):
            compare_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceExternalMemoryRDMAFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceExternalMemoryRDMAFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_EXT_pipeline_properties))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT):
            compare_VkPhysicalDevicePipelinePropertiesFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDevicePipelinePropertiesFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDevicePipelinePropertiesFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_frame_boundary))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT):
            compare_VkPhysicalDeviceFrameBoundaryFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceFrameBoundaryFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceFrameBoundaryFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_multisampled_render_to_single_sampled))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT):
            compare_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_extended_dynamic_state2))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT):
            compare_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_color_write_enable))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT):
            compare_VkPhysicalDeviceColorWriteEnableFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceColorWriteEnableFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceColorWriteEnableFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_primitives_generated_query))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT):
            compare_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VALVE_video_encode_rgb_conversion))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_RGB_CONVERSION_FEATURES_VALVE):
            compare_VkPhysicalDeviceVideoEncodeRgbConversionFeaturesVALVE(error_list, *reinterpret_cast<const VkPhysicalDeviceVideoEncodeRgbConversionFeaturesVALVE*>(supported), *reinterpret_cast<const VkPhysicalDeviceVideoEncodeRgbConversionFeaturesVALVE*>(requested));
            break;
#endif
#if (defined(VK_EXT_image_view_min_lod))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT):
            compare_VkPhysicalDeviceImageViewMinLodFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceImageViewMinLodFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceImageViewMinLodFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_multi_draw))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT):
            compare_VkPhysicalDeviceMultiDrawFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceMultiDrawFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceMultiDrawFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_image_2d_view_of_3d))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT):
            compare_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_shader_tile_image))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT):
            compare_VkPhysicalDeviceShaderTileImageFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderTileImageFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderTileImageFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_opacity_micromap))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT):
            compare_VkPhysicalDeviceOpacityMicromapFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceOpacityMicromapFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceOpacityMicromapFeaturesEXT*>(requested));
            break;
#endif
#if defined(VK_ENABLE_BETA_EXTENSIONS) && (defined(VK_NV_displacement_micromap))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_FEATURES_NV):
            compare_VkPhysicalDeviceDisplacementMicromapFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceDisplacementMicromapFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceDisplacementMicromapFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_HUAWEI_cluster_culling_shader))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_FEATURES_HUAWEI):
            compare_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI(error_list, *reinterpret_cast<const VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI*>(supported), *reinterpret_cast<const VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI*>(requested));
            break;
#endif
#if (defined(VK_EXT_border_color_swizzle))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT):
            compare_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_pageable_device_local_memory))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT):
            compare_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_ARM_scheduling_controls))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_FEATURES_ARM):
            compare_VkPhysicalDeviceSchedulingControlsFeaturesARM(error_list, *reinterpret_cast<const VkPhysicalDeviceSchedulingControlsFeaturesARM*>(supported), *reinterpret_cast<const VkPhysicalDeviceSchedulingControlsFeaturesARM*>(requested));
            break;
#endif
#if (defined(VK_EXT_image_sliced_view_of_3d))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT):
            compare_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_VALVE_descriptor_set_host_mapping))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_SET_HOST_MAPPING_FEATURES_VALVE):
            compare_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE(error_list, *reinterpret_cast<const VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE*>(supported), *reinterpret_cast<const VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE*>(requested));
            break;
#endif
#if (defined(VK_EXT_non_seamless_cube_map))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT):
            compare_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_ARM_render_pass_striped))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_FEATURES_ARM):
            compare_VkPhysicalDeviceRenderPassStripedFeaturesARM(error_list, *reinterpret_cast<const VkPhysicalDeviceRenderPassStripedFeaturesARM*>(supported), *reinterpret_cast<const VkPhysicalDeviceRenderPassStripedFeaturesARM*>(requested));
            break;
#endif
#if (defined(VK_EXT_fragment_density_map_offset))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_EXT):
            compare_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT*>(requested));
            break;
#elif (defined(VK_QCOM_fragment_density_map_offset))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_QCOM):
            compare_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM(error_list, *reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM*>(supported), *reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM*>(requested));
            break;
#endif
#if (defined(VK_NV_copy_memory_indirect))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_NV):
            compare_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceCopyMemoryIndirectFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceCopyMemoryIndirectFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_memory_decompression))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV):
            compare_VkPhysicalDeviceMemoryDecompressionFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceMemoryDecompressionFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceMemoryDecompressionFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_device_generated_commands_compute))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_COMPUTE_FEATURES_NV):
            compare_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_ray_tracing_linear_swept_spheres))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_LINEAR_SWEPT_SPHERES_FEATURES_NV):
            compare_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_linear_color_attachment))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINEAR_COLOR_ATTACHMENT_FEATURES_NV):
            compare_VkPhysicalDeviceLinearColorAttachmentFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceLinearColorAttachmentFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceLinearColorAttachmentFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_EXT_image_compression_control_swapchain))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT):
            compare_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_QCOM_image_processing))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_FEATURES_QCOM):
            compare_VkPhysicalDeviceImageProcessingFeaturesQCOM(error_list, *reinterpret_cast<const VkPhysicalDeviceImageProcessingFeaturesQCOM*>(supported), *reinterpret_cast<const VkPhysicalDeviceImageProcessingFeaturesQCOM*>(requested));
            break;
#endif
#if (defined(VK_EXT_nested_command_buffer))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT):
            compare_VkPhysicalDeviceNestedCommandBufferFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceNestedCommandBufferFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceNestedCommandBufferFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_extended_dynamic_state3))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT):
            compare_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_subpass_merge_feedback))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT):
            compare_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_ARM_tensors))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TENSOR_FEATURES_ARM):
            compare_VkPhysicalDeviceTensorFeaturesARM(error_list, *reinterpret_cast<const VkPhysicalDeviceTensorFeaturesARM*>(supported), *reinterpret_cast<const VkPhysicalDeviceTensorFeaturesARM*>(requested));
            break;
#endif
#if (defined(VK_ARM_tensors))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_TENSOR_FEATURES_ARM):
            compare_VkPhysicalDeviceDescriptorBufferTensorFeaturesARM(error_list, *reinterpret_cast<const VkPhysicalDeviceDescriptorBufferTensorFeaturesARM*>(supported), *reinterpret_cast<const VkPhysicalDeviceDescriptorBufferTensorFeaturesARM*>(requested));
            break;
#endif
#if (defined(VK_EXT_shader_module_identifier))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT):
            compare_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_NV_optical_flow))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV):
            compare_VkPhysicalDeviceOpticalFlowFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceOpticalFlowFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceOpticalFlowFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_EXT_legacy_dithering))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT):
            compare_VkPhysicalDeviceLegacyDitheringFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceLegacyDitheringFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceLegacyDitheringFeaturesEXT*>(requested));
            break;
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR) && (defined(VK_ANDROID_external_format_resolve))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_FEATURES_ANDROID):
            compare_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID(error_list, *reinterpret_cast<const VkPhysicalDeviceExternalFormatResolveFeaturesANDROID*>(supported), *reinterpret_cast<const VkPhysicalDeviceExternalFormatResolveFeaturesANDROID*>(requested));
            break;
#endif
#if (defined(VK_AMD_anti_lag))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ANTI_LAG_FEATURES_AMD):
            compare_VkPhysicalDeviceAntiLagFeaturesAMD(error_list, *reinterpret_cast<const VkPhysicalDeviceAntiLagFeaturesAMD*>(supported), *reinterpret_cast<const VkPhysicalDeviceAntiLagFeaturesAMD*>(requested));
            break;
#endif
#if defined(VK_ENABLE_BETA_EXTENSIONS) && (defined(VK_AMDX_dense_geometry_format))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DENSE_GEOMETRY_FORMAT_FEATURES_AMDX):
            compare_VkPhysicalDeviceDenseGeometryFormatFeaturesAMDX(error_list, *reinterpret_cast<const VkPhysicalDeviceDenseGeometryFormatFeaturesAMDX*>(supported), *reinterpret_cast<const VkPhysicalDeviceDenseGeometryFormatFeaturesAMDX*>(requested));
            break;
#endif
#if (defined(VK_EXT_shader_object))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT):
            compare_VkPhysicalDeviceShaderObjectFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderObjectFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderObjectFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_QCOM_tile_properties))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_PROPERTIES_FEATURES_QCOM):
            compare_VkPhysicalDeviceTilePropertiesFeaturesQCOM(error_list, *reinterpret_cast<const VkPhysicalDeviceTilePropertiesFeaturesQCOM*>(supported), *reinterpret_cast<const VkPhysicalDeviceTilePropertiesFeaturesQCOM*>(requested));
            break;
#endif
#if (defined(VK_SEC_amigo_profiling))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_AMIGO_PROFILING_FEATURES_SEC):
            compare_VkPhysicalDeviceAmigoProfilingFeaturesSEC(error_list, *reinterpret_cast<const VkPhysicalDeviceAmigoProfilingFeaturesSEC*>(supported), *reinterpret_cast<const VkPhysicalDeviceAmigoProfilingFeaturesSEC*>(requested));
            break;
#endif
#if (defined(VK_QCOM_multiview_per_view_viewports))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_VIEWPORTS_FEATURES_QCOM):
            compare_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM(error_list, *reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM*>(supported), *reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM*>(requested));
            break;
#endif
#if (defined(VK_NV_ray_tracing_invocation_reorder))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV):
            compare_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_cooperative_vector))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_FEATURES_NV):
            compare_VkPhysicalDeviceCooperativeVectorFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceCooperativeVectorFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceCooperativeVectorFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_extended_sparse_address_space))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_FEATURES_NV):
            compare_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_EXT_legacy_vertex_attributes))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_FEATURES_EXT):
            compare_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_ARM_shader_core_builtins))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_FEATURES_ARM):
            compare_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM*>(requested));
            break;
#endif
#if (defined(VK_EXT_pipeline_library_group_handles))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT):
            compare_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_dynamic_rendering_unused_attachments))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT):
            compare_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_ARM_data_graph))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DATA_GRAPH_FEATURES_ARM):
            compare_VkPhysicalDeviceDataGraphFeaturesARM(error_list, *reinterpret_cast<const VkPhysicalDeviceDataGraphFeaturesARM*>(supported), *reinterpret_cast<const VkPhysicalDeviceDataGraphFeaturesARM*>(requested));
            break;
#endif
#if (defined(VK_QCOM_multiview_per_view_render_areas))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_RENDER_AREAS_FEATURES_QCOM):
            compare_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM(error_list, *reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM*>(supported), *reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM*>(requested));
            break;
#endif
#if (defined(VK_NV_per_stage_descriptor_set))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PER_STAGE_DESCRIPTOR_SET_FEATURES_NV):
            compare_VkPhysicalDevicePerStageDescriptorSetFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDevicePerStageDescriptorSetFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDevicePerStageDescriptorSetFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_QCOM_image_processing2))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_FEATURES_QCOM):
            compare_VkPhysicalDeviceImageProcessing2FeaturesQCOM(error_list, *reinterpret_cast<const VkPhysicalDeviceImageProcessing2FeaturesQCOM*>(supported), *reinterpret_cast<const VkPhysicalDeviceImageProcessing2FeaturesQCOM*>(requested));
            break;
#endif
#if (defined(VK_QCOM_filter_cubic_weights))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_WEIGHTS_FEATURES_QCOM):
            compare_VkPhysicalDeviceCubicWeightsFeaturesQCOM(error_list, *reinterpret_cast<const VkPhysicalDeviceCubicWeightsFeaturesQCOM*>(supported), *reinterpret_cast<const VkPhysicalDeviceCubicWeightsFeaturesQCOM*>(requested));
            break;
#endif
#if (defined(VK_QCOM_ycbcr_degamma))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_DEGAMMA_FEATURES_QCOM):
            compare_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM(error_list, *reinterpret_cast<const VkPhysicalDeviceYcbcrDegammaFeaturesQCOM*>(supported), *reinterpret_cast<const VkPhysicalDeviceYcbcrDegammaFeaturesQCOM*>(requested));
            break;
#endif
#if (defined(VK_QCOM_filter_cubic_clamp))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_CLAMP_FEATURES_QCOM):
            compare_VkPhysicalDeviceCubicClampFeaturesQCOM(error_list, *reinterpret_cast<const VkPhysicalDeviceCubicClampFeaturesQCOM*>(supported), *reinterpret_cast<const VkPhysicalDeviceCubicClampFeaturesQCOM*>(requested));
            break;
#endif
#if (defined(VK_EXT_attachment_feedback_loop_dynamic_state))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT):
            compare_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT*>(requested));
            break;
#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX) && (defined(VK_QNX_external_memory_screen_buffer))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_SCREEN_BUFFER_FEATURES_QNX):
            compare_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX(error_list, *reinterpret_cast<const VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX*>(supported), *reinterpret_cast<const VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX*>(requested));
            break;
#endif
#if (defined(VK_NV_descriptor_pool_overallocation))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_POOL_OVERALLOCATION_FEATURES_NV):
            compare_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_QCOM_tile_memory_heap))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_MEMORY_HEAP_FEATURES_QCOM):
            compare_VkPhysicalDeviceTileMemoryHeapFeaturesQCOM(error_list, *reinterpret_cast<const VkPhysicalDeviceTileMemoryHeapFeaturesQCOM*>(supported), *reinterpret_cast<const VkPhysicalDeviceTileMemoryHeapFeaturesQCOM*>(requested));
            break;
#endif
#if (defined(VK_NV_raw_access_chains))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAW_ACCESS_CHAINS_FEATURES_NV):
            compare_VkPhysicalDeviceRawAccessChainsFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceRawAccessChainsFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceRawAccessChainsFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_command_buffer_inheritance))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMMAND_BUFFER_INHERITANCE_FEATURES_NV):
            compare_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceCommandBufferInheritanceFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceCommandBufferInheritanceFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_shader_atomic_float16_vector))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT16_VECTOR_FEATURES_NV):
            compare_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_EXT_shader_replicated_composites))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_REPLICATED_COMPOSITES_FEATURES_EXT):
            compare_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_EXT_shader_float8))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT8_FEATURES_EXT):
            compare_VkPhysicalDeviceShaderFloat8FeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceShaderFloat8FeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceShaderFloat8FeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_NV_ray_tracing_validation))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_VALIDATION_FEATURES_NV):
            compare_VkPhysicalDeviceRayTracingValidationFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceRayTracingValidationFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceRayTracingValidationFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_cluster_acceleration_structure))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_FEATURES_NV):
            compare_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceClusterAccelerationStructureFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceClusterAccelerationStructureFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_NV_partitioned_acceleration_structure))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_FEATURES_NV):
            compare_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_EXT_device_generated_commands))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT):
            compare_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_MESA_image_alignment_control))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_FEATURES_MESA):
            compare_VkPhysicalDeviceImageAlignmentControlFeaturesMESA(error_list, *reinterpret_cast<const VkPhysicalDeviceImageAlignmentControlFeaturesMESA*>(supported), *reinterpret_cast<const VkPhysicalDeviceImageAlignmentControlFeaturesMESA*>(requested));
            break;
#endif
#if (defined(VK_EXT_depth_clamp_control))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_CONTROL_FEATURES_EXT):
            compare_VkPhysicalDeviceDepthClampControlFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceDepthClampControlFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceDepthClampControlFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_HUAWEI_hdr_vivid))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HDR_VIVID_FEATURES_HUAWEI):
            compare_VkPhysicalDeviceHdrVividFeaturesHUAWEI(error_list, *reinterpret_cast<const VkPhysicalDeviceHdrVividFeaturesHUAWEI*>(supported), *reinterpret_cast<const VkPhysicalDeviceHdrVividFeaturesHUAWEI*>(requested));
            break;
#endif
#if (defined(VK_NV_cooperative_matrix2))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_FEATURES_NV):
            compare_VkPhysicalDeviceCooperativeMatrix2FeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDeviceCooperativeMatrix2FeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDeviceCooperativeMatrix2FeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_ARM_pipeline_opacity_micromap))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_OPACITY_MICROMAP_FEATURES_ARM):
            compare_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM(error_list, *reinterpret_cast<const VkPhysicalDevicePipelineOpacityMicromapFeaturesARM*>(supported), *reinterpret_cast<const VkPhysicalDevicePipelineOpacityMicromapFeaturesARM*>(requested));
            break;
#endif
#if (defined(VK_EXT_vertex_attribute_robustness))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT):
            compare_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_ARM_format_pack))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FORMAT_PACK_FEATURES_ARM):
            compare_VkPhysicalDeviceFormatPackFeaturesARM(error_list, *reinterpret_cast<const VkPhysicalDeviceFormatPackFeaturesARM*>(supported), *reinterpret_cast<const VkPhysicalDeviceFormatPackFeaturesARM*>(requested));
            break;
#endif
#if (defined(VK_VALVE_fragment_density_map_layered))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_LAYERED_FEATURES_VALVE):
            compare_VkPhysicalDeviceFragmentDensityMapLayeredFeaturesVALVE(error_list, *reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapLayeredFeaturesVALVE*>(supported), *reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapLayeredFeaturesVALVE*>(requested));
            break;
#endif
#if defined(VK_ENABLE_BETA_EXTENSIONS) && (defined(VK_NV_present_metering))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_METERING_FEATURES_NV):
            compare_VkPhysicalDevicePresentMeteringFeaturesNV(error_list, *reinterpret_cast<const VkPhysicalDevicePresentMeteringFeaturesNV*>(supported), *reinterpret_cast<const VkPhysicalDevicePresentMeteringFeaturesNV*>(requested));
            break;
#endif
#if (defined(VK_EXT_zero_initialize_device_memory))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_DEVICE_MEMORY_FEATURES_EXT):
            compare_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT*>(requested));
            break;
#endif
#if (defined(VK_SEC_pipeline_cache_incremental_mode))
            case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CACHE_INCREMENTAL_MODE_FEATURES_SEC):
            compare_VkPhysicalDevicePipelineCacheIncrementalModeFeaturesSEC(error_list, *reinterpret_cast<const VkPhysicalDevicePipelineCacheIncrementalModeFeaturesSEC*>(supported), *reinterpret_cast<const VkPhysicalDevicePipelineCacheIncrementalModeFeaturesSEC*>(requested));
            break;
#endif
#if (defined(VK_KHR_acceleration_structure))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR):
            compare_VkPhysicalDeviceAccelerationStructureFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceAccelerationStructureFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceAccelerationStructureFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_ray_tracing_pipeline))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR):
            compare_VkPhysicalDeviceRayTracingPipelineFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceRayTracingPipelineFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceRayTracingPipelineFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_KHR_ray_query))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR):
            compare_VkPhysicalDeviceRayQueryFeaturesKHR(error_list, *reinterpret_cast<const VkPhysicalDeviceRayQueryFeaturesKHR*>(supported), *reinterpret_cast<const VkPhysicalDeviceRayQueryFeaturesKHR*>(requested));
            break;
#endif
#if (defined(VK_EXT_mesh_shader))
        case(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT):
            compare_VkPhysicalDeviceMeshShaderFeaturesEXT(error_list, *reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesEXT*>(supported), *reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesEXT*>(requested));
            break;
#endif
        default:
            break;
    }
*/
}

void merge_feature_struct(VkStructureType sType, void *current, const void *merge_in) {

}

} // end namespace lvk
