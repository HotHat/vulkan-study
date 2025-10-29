//
// Created by admin on 2025/10/27.
//

#ifndef LVK_FUNCTIONS_H
#define LVK_FUNCTIONS_H
#include <vulkan/vulkan.h>
#include <vector>
#include <assert.h>
#include <string>

namespace lvk {


inline const uint32_t QUEUE_INDEX_MAX_VALUE = UINT32_MAX;

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

// Helper for robustly executing the two-call pattern
template <typename T, typename F, typename... Ts> auto get_vector(std::vector<T>& out, F&& f, Ts&&... ts) -> VkResult {
    uint32_t count = 0;
    VkResult err;
    do {
        err = f(ts..., &count, nullptr);
        if (err != VK_SUCCESS) {
            return err;
        };
        out.resize(count);
        err = f(ts..., &count, out.data());
        out.resize(count);
    } while (err == VK_INCOMPLETE);
    return err;
}

template <typename T, typename F, typename... Ts> auto get_vector_noerror(F&& f, Ts&&... ts) -> std::vector<T> {
    uint32_t count = 0;
    std::vector<T> results;
    f(ts..., &count, nullptr);
    results.resize(count);
    f(ts..., &count, results.data());
    results.resize(count);
    return results;
}

std::vector<std::string> find_unsupported_extensions_in_list(
        std::vector<std::string> const& available_extensions, std::vector<std::string> const& required_extensions);

// Finds the first queue which supports the desired operations. Returns QUEUE_INDEX_MAX_VALUE if none is found
uint32_t get_first_queue_index(std::vector<VkQueueFamilyProperties> const& families, VkQueueFlags desired_flags);
// Finds the queue which is separate from the graphics queue and has the desired flag and not the
// undesired flag, but will select it if no better options are available compute support. Returns
// QUEUE_INDEX_MAX_VALUE if none is found.
uint32_t get_separate_queue_index(
        std::vector<VkQueueFamilyProperties> const& families, VkQueueFlags desired_flags, VkQueueFlags undesired_flags);

// finds the first queue which supports only the desired flag (not graphics or transfer). Returns QUEUE_INDEX_MAX_VALUE if none is found.
uint32_t get_dedicated_queue_index(
        std::vector<VkQueueFamilyProperties> const& families, VkQueueFlags desired_flags, VkQueueFlags undesired_flags);

// finds the first queue which supports presenting. returns QUEUE_INDEX_MAX_VALUE if none is found
uint32_t get_present_queue_index(
        VkPhysicalDevice const phys_device, VkSurfaceKHR const surface, std::vector<VkQueueFamilyProperties> const& families);

template<typename T>
T get_instance_proc_addr(VkInstance instance, const char *fun) {
    auto tmp = vkGetInstanceProcAddr(instance, fun);
    return reinterpret_cast<T>(tmp);
}

template<typename T>
T get_device_proc_addr(VkDevice device, const char *fun) {
    auto tmp = vkGetDeviceProcAddr(device, fun);
    return reinterpret_cast<T>(tmp);
}

} // end of namespace lvk
#endif //LVK_FUNCTIONS_H
