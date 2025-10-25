//
// Created by admin on 2025/10/24.
//

#include "LkInstance.h"
#include <iostream>


static VkBool32 DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
    std::cerr << "validation layer: \n"
            << "\t\t" << "messageIdName: " << pCallbackData->pMessageIdName << "\n"
            << "\t\t" << "messageIdNumber: " << pCallbackData->messageIdNumber << "\n"
            << "\t\t" << "message: " << pCallbackData->pMessage << "\n"
            << "\t\t" << "queueLabelCount: " << pCallbackData->queueLabelCount << "\n"
            << "\t\t" << "cmdBufLabelCount: " << pCallbackData->cmdBufLabelCount << "\n"
            << std::endl;

    return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                          const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void LkInstance::CreateInstance(const std::vector<const char *> &validationLayers,
                                const std::vector<const char *> &extensions) {
    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Hello Vulkan",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_1
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = DebugCallback,
    };

    VkInstanceCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .pApplicationInfo = &appInfo,
    };

    // enable validation
    if (m_enableValidation) {
        info.pNext = &debugCreateInfo;
        info.pApplicationInfo = &appInfo;
        info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        info.ppEnabledLayerNames = validationLayers.data();
        info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        info.ppEnabledExtensionNames = extensions.data();
    }


    VkResult result = vkCreateInstance(&info, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    if (m_enableValidation) {
        CreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger);
    }
    //
    LayerProperties();
    //
    PhysicalDevices();
}

void LkInstance::EnableValidation() {
    m_enableValidation = true;
}

void LkInstance::PrintVersion() {
    uint32_t appVersion;
    vkEnumerateInstanceVersion(&appVersion);
    uint32_t major, minor, patch;
    major = VK_VERSION_MAJOR(appVersion);
    minor = VK_VERSION_MINOR(appVersion);
    patch = VK_VERSION_PATCH(appVersion);

    std::cout << "vulkan version major:" << major << ", minor: " << minor << ", patch: " << patch << "\n";
}

VkInstance LkInstance::GetInstance() {
    return m_instance;
}

uint32_t LkInstance::PickPhysicDevice() {
    uint8_t index = -1;
    int value = 0;
    if (m_deviceQueueFamilyProperties.empty()) {
        throw std::runtime_error("no device queue family properties available");
    }

    for (auto it = m_deviceQueueFamilyProperties.begin();
         it != m_deviceQueueFamilyProperties.end(); ++it) {

        for (const auto p: it->second) {
            int v = 0;
            if (p.queueFlags & VK_QUEUE_GRAPHICS_BIT > 0) {
                v += 1;
            }
            if (p.queueFlags & VK_QUEUE_COMPUTE_BIT > 0) {
                v += 2;
            }
            if (v >= value) {
                index = it->first;
            }
        }
    }
    if (index == -1) {
        throw std::runtime_error("no device queue family with VK_QUEUE_GRAPHICS_BIT");
    }

    return index;
}

LkInstance::~LkInstance() {
    if (m_instance != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);

        // vkDestroyDevice(device, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }
}

void LkInstance::LayerProperties() {
    uint32_t count;
    // std::vector<>
    vkEnumerateInstanceLayerProperties(&count, nullptr);

    if (count > 0) {
        m_layerProperties.resize(count);

        vkEnumerateInstanceLayerProperties(&count, m_layerProperties.data());

        VK_DEBUG_INFO(
            m_enableValidation,
            {
            for (int i = 0; i < count; ++i) {
            std::cout << "--------------" << i << "--------------\n";
            std::cout << "layer name: " << m_layerProperties[i].layerName << "\n";
            std::cout << "specVersion: " << m_layerProperties[i].specVersion << "\n";
            std::cout << "implementationVersion: " << m_layerProperties[i].implementationVersion << "\n";
            std::cout << "description: " << m_layerProperties[i].description << "\n";
            }
            }
        );
    }

    LayerExtensionProperties();
}

void LkInstance::LayerExtensionProperties() {
    for (const auto layer: m_layerProperties) {
        uint32_t count;
        vkEnumerateInstanceExtensionProperties(layer.layerName, &count, nullptr);
        std::vector<VkExtensionProperties> props;
        if (count > 0) {
            props.resize(count);
            vkEnumerateInstanceExtensionProperties(layer.layerName, &count, props.data());
            VK_DEBUG_INFO(
                m_enableValidation,
                {
                std::cout << "--------------" << layer.layerName << "--------------\n";
                for (int i = 0; i < count; ++i) {
                std::cout << "extension name: " << props[i].extensionName << "\n";
                std::cout << "specVersion: " << props[i].specVersion << "\n";
                }
                }
            );
        }

        m_layerExtensionProperties[layer.layerName] = props;
    }
}

void LkInstance::PhysicalDevices() {
    uint32_t count;
    vkEnumeratePhysicalDevices(m_instance, &count, nullptr);
    if (count) {
        m_physicalDevices.resize(count);

        vkEnumeratePhysicalDevices(m_instance, &count, m_physicalDevices.data());

        int index = 0;
        for (const auto device: m_physicalDevices) {
            VK_DEBUG_INFO(m_enableValidation, {
                          std::cout << "physical device index: " << index << "\n";
                          });
            PhysicalDeviceProperties(device);
            //
            PhysicalDeviceQueueFamilyProperties(index, device);
            //
            PhysicalDeviceExtensionProperties(index, device);
            index++;
        }
    }
}

void LkInstance::PhysicalDeviceProperties(const VkPhysicalDevice &physicalDevice) const {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice, &props);

    VK_DEBUG_INFO(m_enableValidation, {
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
                  });
}

void LkInstance::PhysicalDeviceQueueFamilyProperties(uint8_t index, const VkPhysicalDevice &physicalDevice) {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

    std::vector<VkQueueFamilyProperties> props;
    if (count > 0) {
        VK_DEBUG_INFO(m_enableValidation, {
                      std::cout << "Queue Family Properties size: " << count << "\n";
                      });
        props.resize(count);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, props.data());
        VK_DEBUG_INFO(
            m_enableValidation,
            {
            for (const auto p: props) {
            std::cout << "queueFlags: " << p.queueFlags << "\n";
            std::cout << "\t\t" << "VK_QUEUE_GRAPHICS_BIT : " << (p.queueFlags & VK_QUEUE_GRAPHICS_BIT) <<
            "\n";
            std::cout << "\t\t" << "VK_QUEUE_COMPUTE_BIT  : " << (p.queueFlags & VK_QUEUE_COMPUTE_BIT) << "\n"
            ;
            std::cout << "\t\t" << "VK_QUEUE_TRANSFER_BIT : " << (p.queueFlags & VK_QUEUE_TRANSFER_BIT) <<
            "\n";
            std::cout << "\t\t" << "VK_QUEUE_VIDEO_DECODE_BIT_KHR  : " << (p.queueFlags &
                VK_QUEUE_VIDEO_DECODE_BIT_KHR)
            << "\n";
            std::cout << "queueCount: " << p.queueCount << "\n";
            std::cout << "timestampValidBits: " << p.timestampValidBits << "\n";
            std::cout << "minImageTransferGranularity.width: " << p.minImageTransferGranularity.width << "\n";
            std::cout << "minImageTransferGranularity.height: " << p.minImageTransferGranularity.height <<
            "\n";
            std::cout << "minImageTransferGranularity.depth: " << p.minImageTransferGranularity.depth << "\n";
            }
            }
        );
    }

    m_deviceQueueFamilyProperties[index] = std::move(props);
}

void LkInstance::PhysicalDeviceExtensionProperties(uint8_t index, VkPhysicalDevice const &physicalDevice) {
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> props;
    VK_DEBUG_INFO(
        m_enableValidation,
        {
        std::cout << "device: " << index << "extension properties\n";
        }
    );
    if (count > 0) {
        props.resize(count);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, props.data());
        VK_DEBUG_INFO(
            m_enableValidation,
            {
            for (const auto p: props) {
            std::cout << "extensionName: " << p.extensionName << "\n";
            std::cout << "specVersion: " << p.specVersion << "\n";
            }
            }
        );
    }
}
