//
// Created by admin on 2025/10/23.
//

#include "VkInfo.h"
#include <vulkan/vulkan.h>
#include <iostream>
#include <GLFW/glfw3.h>

void VkInfo::Run() {

   Version();
   LayerProperties();

   CreateInstance();
}



void VkInfo::LayerProperties() {
    uint32_t count;
    // std::vector<>
    vkEnumerateInstanceLayerProperties(&count, nullptr);

    if (count > 0) {

        layerProperties.resize(count);

        vkEnumerateInstanceLayerProperties(&count, layerProperties.data());
        for (int i = 0; i < count; ++i) {
            std::cout << "--------------" << i << "--------------\n";
            std::cout << "layer name: " << layerProperties[i].layerName << "\n";
            std::cout << "specVersion: " << layerProperties[i].specVersion << "\n";
            std::cout << "implementationVersion: " << layerProperties[i].implementationVersion << "\n";
            std::cout << "description: " << layerProperties[i].description << "\n";
        }
    }

    LayerExtensionProperties();
}

void VkInfo::Version() {
    uint32_t appVersion;
    vkEnumerateInstanceVersion(&appVersion);

    uint32_t major, minor, patch;
    major = VK_VERSION_MAJOR(appVersion);
    minor = VK_VERSION_MINOR(appVersion);
    patch = VK_VERSION_PATCH(appVersion);

    std::cout << "vulkan version major:" << major << ", minor: " << minor << ", patch: " << patch << "\n";
}

static  VkBool32 DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: \n"
            << "\t\t" << "messageIdName: " << pCallbackData->pMessageIdName << "\n"
            << "\t\t" << "messageIdNumber: " << pCallbackData->messageIdNumber << "\n"
            << "\t\t" << "message: " << pCallbackData->pMessage << "\n"
            << "\t\t" << "queueLabelCount: " << pCallbackData->queueLabelCount << "\n"
            << "\t\t" << "cmdBufLabelCount: " << pCallbackData->cmdBufLabelCount << "\n"
            << std::endl;

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void VkInfo::CreateInstance() {

    const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
    };

    // std::vector<const char*> extensions {
    //         VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    // };
    std::vector<const char*> extensions;
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    VkApplicationInfo appInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Hello Vulkan",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_1
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = DebugCallback,
    };

    VkInstanceCreateInfo info {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &debugCreateInfo,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
        .ppEnabledLayerNames = validationLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    };


    VkResult result = vkCreateInstance(&info, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!" );
    }

    // debug
    CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger);
}

void VkInfo::LayerExtensionProperties() {

    for (const auto layer : layerProperties) {
        uint32_t count;
        vkEnumerateInstanceExtensionProperties(layer.layerName, &count, nullptr);
        std::vector<VkExtensionProperties> props;
        if (count > 0) {
            props.resize(count);
            vkEnumerateInstanceExtensionProperties(layer.layerName, &count, props.data());

            std::cout << "--------------" << layer.layerName << "--------------\n";
            for (int i = 0; i < count; ++i) {
                std::cout << "extension name: " << props[i].extensionName << "\n";
                std::cout << "specVersion: " << props[i].specVersion << "\n";
            }
        }

        layerExtensionProperties[layer.layerName] = props;
    }

}

VkInfo::~VkInfo() {
    if (instance != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        vkDestroyInstance(instance, nullptr);
    }
}
