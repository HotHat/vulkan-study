//
// Created by admin on 2025/10/23.
//

#include "VkInfo.h"

#include <iostream>
#include <GLFW/glfw3.h>

void VkInfo::Run() {
    instance_.PrintVersion();
    instance_.EnableValidation();
    instance_.CreateInstance(
        {
            "VK_LAYER_KHRONOS_validation"
        }, {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
           "VK_KHR_surface"
        }
    );

    uint32_t deviceIndex = instance_.PickPhysicDevice();

    std::cout << "Device index: " << deviceIndex << std::endl;
    // Version();
    // LayerProperties();
    //
    // CreateInstance();
    // PhysicalDevices();
    //
    // CreateDevice(0);
}

void VkInfo::CreateSurface() {
    if (glfwCreateWindowSurface(instance_.GetInstance(), window_, nullptr, &surface_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}


/*
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

    std::vector<const char*> extensions {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            "VK_KHR_surface"
    };
    // std::vector<const char*> extensions;
    // extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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
            .messageSeverity = // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
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

        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
    }
}

void VkInfo::PhysicalDevices() {
    uint32_t count;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (count) {
        physicalDevices.resize(count);

        vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data());

        int index = 0;
        for (const auto device: physicalDevices) {
            std::cout << "physical device index: " << index << "\n";
            PhysicalDeviceProperties(device);
            //
            PhysicalDeviceQueueFamilyProperties(index, device);
            //
            PhysicalDeviceExtensionProperties(index, device);
            index++;
        }
    }
}

void VkInfo::PhysicalDeviceProperties(const VkPhysicalDevice & physicalDevice) {

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice, &props);

    std::cout << "apiVersion: " << props.apiVersion << "\n";
    std::cout << "driverVersion: " << props.driverVersion << "\n";
    std::cout << "vendorID: " << props.vendorID << "\n";
    std::cout << "deviceID: " << props.deviceID << "\n";
    std::cout << "deviceType: " << props.deviceType << "\n";
    std::cout << "deviceName: " << props.deviceName << "\n";
    std::cout << "pipelineCacheUUID: " << props.pipelineCacheUUID << "\n";
    std::cout << "limits.maxMemoryAllocationCount: " << props.limits.maxMemoryAllocationCount << "\n";
    std::cout << "limits.maxVertexInputBindings: " << props.limits.maxVertexInputBindings << "\n";
    std::cout << "limits.maxTexelBufferElements: " << props.limits.maxTexelBufferElements << "\n";
    std::cout << "limits.maxUniformBufferRange: " << props.limits.maxUniformBufferRange << "\n";
}

void VkInfo::PhysicalDeviceQueueFamilyProperties(uint8_t index, const VkPhysicalDevice & physicalDevice) {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

    std::vector<VkQueueFamilyProperties> props;
    if (count > 0) {
        std::cout << "Queue Family Properties size: " << count << "\n";
        props.resize(count);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, props.data());

        for (const auto p: props) {
            std::cout << "queueFlags: " << p.queueFlags << "\n";
            std::cout << "\t\t" << "VK_QUEUE_GRAPHICS_BIT : " << (p.queueFlags & VK_QUEUE_GRAPHICS_BIT)  << "\n";
            std::cout << "\t\t" << "VK_QUEUE_COMPUTE_BIT  : " << (p.queueFlags & VK_QUEUE_COMPUTE_BIT )  << "\n";
            std::cout << "\t\t" << "VK_QUEUE_TRANSFER_BIT : " << (p.queueFlags & VK_QUEUE_TRANSFER_BIT)  << "\n";
            std::cout << "\t\t" << "VK_QUEUE_VIDEO_DECODE_BIT_KHR  : " << (p.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR )  << "\n";
            std::cout << "queueCount: " << p.queueCount << "\n";
            std::cout << "timestampValidBits: " << p.timestampValidBits << "\n";
            std::cout << "minImageTransferGranularity.width: " << p.minImageTransferGranularity.width << "\n";
            std::cout << "minImageTransferGranularity.height: " << p.minImageTransferGranularity.height << "\n";
            std::cout << "minImageTransferGranularity.depth: " << p.minImageTransferGranularity.depth << "\n";
        }
    }

    deviceQueueFamilyProperties[index] = std::move(props);
}

void VkInfo::CreateDevice(int physicalDeviceIndex) {

    VkPhysicalDevice physicalDevice = physicalDevices[physicalDeviceIndex];
    if (deviceQueueFamilyProperties[physicalDeviceIndex].empty()) {
        throw std::runtime_error("physicalDeviceIndex: " + std::to_string(physicalDeviceIndex) + ", No device queue family found");
    }
    uint32_t queueFamilyIndex = 0;

    std::vector<const char *> extensions {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
    };

    VkDeviceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
        .pEnabledFeatures = nullptr
    };

    VkResult result = vkCreateDevice(physicalDevice, &info, nullptr, &device);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create device!" );
    } else {
        std::cout << "logic device create success" << "\n";
    }
}

void VkInfo::PhysicalDeviceExtensionProperties(uint8_t index, VkPhysicalDevice const &physicalDevice) {
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> props;

    std::cout << "device: " << index << "extension properties\n";
    if (count > 0) {
        props.resize(count);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, props.data());

        for (const auto p: props) {
            std::cout << "extensionName: " << p.extensionName << "\n";
            std::cout << "specVersion: " << p.specVersion << "\n";
        }
    }


}
*/
