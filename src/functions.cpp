//
// Created by admin on 2025/10/27.
//

#include <cassert>
#include "functions.h"


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

} // end namespace lvk
