//
// Created by admin on 2025/10/27.
//

#ifndef LVK_FUNCTIONS_H
#define LVK_FUNCTIONS_H
#include <vulkan/vulkan.h>
#include <vector>
#include <assert.h>

namespace lvk {


const char *to_string_message_severity(VkDebugUtilsMessageSeverityFlagBitsEXT s);

const char *to_string_message_type(VkDebugUtilsMessageTypeFlagsEXT s);

// Default debug messenger
// Feel free to copy-paste it into your own code, change it as needed, then call `set_debug_callback()` to use that instead
inline VkBool32 default_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                       void *) {
    auto ms = to_string_message_severity(messageSeverity);
    auto mt = to_string_message_type(messageType);
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        printf("[%s: %s] - %s\n%s\n", ms, mt, pCallbackData->pMessageIdName, pCallbackData->pMessage);
    } else {
        printf("[%s: %s]\n%s\n", ms, mt, pCallbackData->pMessage);
    }

    return VK_FALSE; // Applications must return false here (Except Validation, if return true, will skip calling to driver)
}

void destroy_debug_utils_messenger(
        VkInstance instance, VkDebugUtilsMessengerEXT messenger,
        VkAllocationCallbacks *allocation_callbacks = nullptr);

template <typename T> void setup_next_chain(T& structure, std::vector<void*> const& structs) {
    structure.pNext = nullptr;
    if (structs.empty()) return;
    for (size_t i = 0; i < structs.size() - 1; i++) {
        VkBaseOutStructure out_structure{};
        memcpy(&out_structure, structs.at(i), sizeof(VkBaseOutStructure));
#if !defined(NDEBUG)
        assert(out_structure.sType != VK_STRUCTURE_TYPE_APPLICATION_INFO);
#endif
        out_structure.pNext = static_cast<VkBaseOutStructure*>(structs.at(i + 1));
        memcpy(structs.at(i), &out_structure, sizeof(VkBaseOutStructure));
    }
    VkBaseOutStructure out_structure{};
    memcpy(&out_structure, structs.back(), sizeof(VkBaseOutStructure));
    out_structure.pNext = nullptr;
#if !defined(NDEBUG)
    assert(out_structure.sType != VK_STRUCTURE_TYPE_APPLICATION_INFO);
#endif
    memcpy(structs.back(), &out_structure, sizeof(VkBaseOutStructure));
    structure.pNext = structs.at(0);
}

VkResult create_debug_utils_messenger(VkInstance instance,
                                      PFN_vkDebugUtilsMessengerCallbackEXT debug_callback,
                                      VkDebugUtilsMessageSeverityFlagsEXT severity,
                                      VkDebugUtilsMessageTypeFlagsEXT type,
                                      void* user_data_pointer,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger,
                                      VkAllocationCallbacks* allocation_callbacks);
} // end of namespace lvk
#endif //LVK_FUNCTIONS_H
