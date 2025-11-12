//
// Created by admin on 2025/10/27.
//

#ifndef LVK_FUNCTIONS_H
#define LVK_FUNCTIONS_H
#include <vulkan/vulkan.h>
#include <vector>
#include <assert.h>
#include <string>
#include <mutex>

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

class VulkanFunctions {
private:
    std::mutex init_mutex;
    bool initialized = false;

    std::mutex instance_functions_mutex;
    bool instance_functions_initialized = false;


public:
    bool init_vulkan_funcs() {
        std::lock_guard<std::mutex> lg(init_mutex);
        if (initialized) {
            return true;
        }
        //
        ptr_vkGetInstanceProcAddr = vkGetInstanceProcAddr;

        fp_vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
                ptr_vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceExtensionProperties"));
        fp_vkEnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(
                ptr_vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceLayerProperties"));
        fp_vkEnumerateInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
                ptr_vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"));
        fp_vkCreateInstance =
                reinterpret_cast<PFN_vkCreateInstance>(ptr_vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance"));
        initialized = true;
        return true;
    }

    template <typename T> void get_inst_proc_addr(T& out_ptr, const char* func_name) {
        out_ptr = reinterpret_cast<T>(ptr_vkGetInstanceProcAddr(instance, func_name));
    }

    template <typename T> void get_device_proc_addr(VkDevice device, T& out_ptr, const char* func_name) {
        out_ptr = reinterpret_cast<T>(fp_vkGetDeviceProcAddr(device, func_name));
    }

    PFN_vkGetInstanceProcAddr ptr_vkGetInstanceProcAddr = nullptr;
    VkInstance instance = nullptr;

    PFN_vkEnumerateInstanceExtensionProperties fp_vkEnumerateInstanceExtensionProperties = nullptr;
    PFN_vkEnumerateInstanceLayerProperties fp_vkEnumerateInstanceLayerProperties = nullptr;
    PFN_vkEnumerateInstanceVersion fp_vkEnumerateInstanceVersion = nullptr;
    PFN_vkCreateInstance fp_vkCreateInstance = nullptr;

    PFN_vkDestroyInstance fp_vkDestroyInstance = nullptr;
    PFN_vkCreateDebugUtilsMessengerEXT fp_vkCreateDebugUtilsMessengerEXT = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT fp_vkDestroyDebugUtilsMessengerEXT = nullptr;
    PFN_vkEnumeratePhysicalDevices fp_vkEnumeratePhysicalDevices = nullptr;
    PFN_vkGetPhysicalDeviceFeatures fp_vkGetPhysicalDeviceFeatures = nullptr;
    PFN_vkGetPhysicalDeviceFeatures2 fp_vkGetPhysicalDeviceFeatures2 = nullptr;
    PFN_vkGetPhysicalDeviceFeatures2KHR fp_vkGetPhysicalDeviceFeatures2KHR = nullptr;
    PFN_vkGetPhysicalDeviceProperties fp_vkGetPhysicalDeviceProperties = nullptr;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties fp_vkGetPhysicalDeviceQueueFamilyProperties = nullptr;
    PFN_vkGetPhysicalDeviceMemoryProperties fp_vkGetPhysicalDeviceMemoryProperties = nullptr;
    PFN_vkEnumerateDeviceExtensionProperties fp_vkEnumerateDeviceExtensionProperties = nullptr;

    PFN_vkCreateDevice fp_vkCreateDevice = nullptr;
    PFN_vkGetDeviceProcAddr fp_vkGetDeviceProcAddr = nullptr;

    PFN_vkDestroySurfaceKHR fp_vkDestroySurfaceKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR fp_vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fp_vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fp_vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fp_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;

    void init_instance_funcs(VkInstance inst);

    void deinit();
};

VulkanFunctions& vulkan_functions();

} // end of namespace lvk
#endif //LVK_FUNCTIONS_H
