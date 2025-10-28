//
// Created by admin on 2025/10/27.
//

#include <cassert>
#include "functions.h"
#include <algorithm>

namespace lvk {

const char* to_string_message_severity(VkDebugUtilsMessageSeverityFlagBitsEXT s) {
    switch (s) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            return "VERBOSE";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            return "ERROR";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            return "WARNING";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            return "INFO";
        default:
            return "UNKNOWN";
    }
}
const char* to_string_message_type(VkDebugUtilsMessageTypeFlagsEXT s) {
    if (s == 7) return "General | Validation | Performance";
    if (s == 6) return "Validation | Performance";
    if (s == 5) return "General | Performance";
    if (s == 4 /*VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT*/) return "Performance";
    if (s == 3) return "General | Validation";
    if (s == 2 /*VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT*/) return "Validation";
    if (s == 1 /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT*/) return "General";
    return "Unknown";
}



VkResult create_debug_utils_messenger(VkInstance instance,
                                      PFN_vkDebugUtilsMessengerCallbackEXT debug_callback,
                                      VkDebugUtilsMessageSeverityFlagsEXT severity,
                                      VkDebugUtilsMessageTypeFlagsEXT type,
                                      void* user_data_pointer,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger,
                                      VkAllocationCallbacks* allocation_callbacks) {

    if (debug_callback == nullptr) debug_callback = default_debug_callback;
    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
    messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerCreateInfo.pNext = nullptr;
    messengerCreateInfo.messageSeverity = severity;
    messengerCreateInfo.messageType = type;
    messengerCreateInfo.pfnUserCallback = debug_callback;
    messengerCreateInfo.pUserData = user_data_pointer;

    PFN_vkVoidFunction tmp;
    tmp = vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    auto func =reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(tmp);
    if (func != nullptr) {
        return func(instance, &messengerCreateInfo, allocation_callbacks, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroy_debug_utils_messenger(
        VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* allocation_callbacks) {
    PFN_vkVoidFunction tmp;
    tmp = vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    auto func =reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(tmp);
    if (func != nullptr) {
        return func(instance, debugMessenger, allocation_callbacks);
    }
}

std::vector<std::string> find_unsupported_extensions_in_list(
        std::vector<std::string> const& available_extensions, std::vector<std::string> const& required_extensions) {
        std::vector<std::string> unavailable_extensions;

        for (auto& req_ext : required_extensions) {
            if (!std::binary_search(available_extensions.begin(), available_extensions.end(), req_ext)) {
                unavailable_extensions.push_back(req_ext);
            }
        }
        return unavailable_extensions;
}

// Finds the first queue which supports the desired operations. Returns QUEUE_INDEX_MAX_VALUE if none is found
uint32_t get_first_queue_index(std::vector<VkQueueFamilyProperties> const& families, VkQueueFlags desired_flags) {
    for (uint32_t i = 0; i < static_cast<uint32_t>(families.size()); i++) {
        if ((families[i].queueFlags & desired_flags) == desired_flags) return i;
    }
    return QUEUE_INDEX_MAX_VALUE;
}
// Finds the queue which is separate from the graphics queue and has the desired flag and not the
// undesired flag, but will select it if no better options are available compute support. Returns
// QUEUE_INDEX_MAX_VALUE if none is found.
uint32_t get_separate_queue_index(
        std::vector<VkQueueFamilyProperties> const& families, VkQueueFlags desired_flags, VkQueueFlags undesired_flags) {
    uint32_t index = QUEUE_INDEX_MAX_VALUE;
    for (uint32_t i = 0; i < static_cast<uint32_t>(families.size()); i++) {
        if ((families[i].queueFlags & desired_flags) == desired_flags && ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
            if ((families[i].queueFlags & undesired_flags) == 0) {
                return i;
            } else {
                index = i;
            }
        }
    }
    return index;
}

// finds the first queue which supports only the desired flag (not graphics or transfer). Returns QUEUE_INDEX_MAX_VALUE if none is found.
uint32_t get_dedicated_queue_index(
        std::vector<VkQueueFamilyProperties> const& families, VkQueueFlags desired_flags, VkQueueFlags undesired_flags) {
    for (uint32_t i = 0; i < static_cast<uint32_t>(families.size()); i++) {
        if ((families[i].queueFlags & desired_flags) == desired_flags &&
            (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 && (families[i].queueFlags & undesired_flags) == 0)
            return i;
    }
    return QUEUE_INDEX_MAX_VALUE;
}

// finds the first queue which supports presenting. returns QUEUE_INDEX_MAX_VALUE if none is found
uint32_t get_present_queue_index(
        VkPhysicalDevice phys_device, VkSurfaceKHR surface, std::vector<VkQueueFamilyProperties> const& families) {
    for (uint32_t i = 0; i < static_cast<uint32_t>(families.size()); i++) {
        VkBool32 presentSupport = VK_FALSE;
        if (surface != VK_NULL_HANDLE) {
            VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(phys_device, i, surface, &presentSupport);
            if (res != VK_SUCCESS) return QUEUE_INDEX_MAX_VALUE; // TODO: determine if this should fail another way
        }
        if (presentSupport == VK_TRUE) return i;
    }

    return QUEUE_INDEX_MAX_VALUE;
}

} // end namespace lvk
