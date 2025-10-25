//
// Created by admin on 2025/10/24.
//

#ifndef LYH_LK_INSTANCE_H
#define LYH_LK_INSTANCE_H

#include <vulkan/vulkan.h>
#include <vector>
#include <map>
#include <string>
#include "vk.h"

class LkInstance {
public:
    void CreateInstance(const std::vector<const char*> &validationLayers, const std::vector<const char*> &extensions);
    void EnableValidation();
    void PrintVersion();
    VkInstance GetInstance();

    uint32_t PickPhysicDevice();

    ~LkInstance();
private:
    void LayerProperties();
    void LayerExtensionProperties();
    void PhysicalDevices();
    void PhysicalDeviceProperties(const VkPhysicalDevice & physicalDevice) const;
    void PhysicalDeviceQueueFamilyProperties(uint8_t index, const VkPhysicalDevice & physicalDevice);
    void PhysicalDeviceExtensionProperties(uint8_t index, const VkPhysicalDevice & physicalDevice);

private:
    bool m_enableValidation = false;

    std::vector<VkLayerProperties> m_layerProperties;
    std::map<std::string, std::vector<VkExtensionProperties>> m_layerExtensionProperties;
    VkInstance m_instance = VK_NULL_HANDLE;

    //
    std::vector<VkPhysicalDevice> m_physicalDevices;
    std::map<uint8_t, std::vector<VkQueueFamilyProperties>> m_deviceQueueFamilyProperties;


    //
    VkDebugUtilsMessengerEXT m_debugMessenger = nullptr;
};


#endif //LYH_LK_INSTANCE_H
