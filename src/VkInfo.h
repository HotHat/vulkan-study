//
// Created by admin on 2025/10/23.
//

#ifndef VULKAN_VKINFO_H
#define VULKAN_VKINFO_H

#include <vulkan/vulkan.h>
#include <vector>
#include <map>
#include <string>
#include <GLFW/glfw3.h>

#include "LkInstance.h"

class VkInfo {

public:

    void Run();
    // ~VkInfo();
    void CreateSurface();

private:
    LkInstance instance_{};
    GLFWwindow* window_ = nullptr;
    VkSurfaceKHR  surface_ = VK_NULL_HANDLE;
    // void Version();
    // void LayerProperties();
    //
    // void LayerExtensionProperties();
    //
    //
    // void CreateInstance();
    //
    // void PhysicalDevices();
    // void PhysicalDeviceProperties(const VkPhysicalDevice & physicalDevice);
    // void PhysicalDeviceQueueFamilyProperties(uint8_t index, const VkPhysicalDevice & physicalDevice);
    // void PhysicalDeviceExtensionProperties(uint8_t index, const VkPhysicalDevice & physicalDevice);
    //
    // void CreateDevice(int physicalDeviceIndex);

private:
    // std::vector<VkLayerProperties> layerProperties;
    // std::map<std::string, std::vector<VkExtensionProperties>> layerExtensionProperties;
    // VkInstance instance = VK_NULL_HANDLE;
    //
    // //
    // std::vector<VkPhysicalDevice> physicalDevices;
    // std::map<uint8_t, std::vector<VkQueueFamilyProperties>> deviceQueueFamilyProperties;
    // // std::map<uint8_t, std::vector<VkExtensionProperties>> deviceExtension;
    //
    // //
    // VkDevice device = nullptr;
    //
    // VkDebugUtilsMessengerEXT debugMessenger = nullptr;
};


#endif //VULKAN_VKINFO_H
