//
// Created by admin on 2025/10/27.
//

#ifndef LVK_SYSTEM_INFO_H
#define LVK_SYSTEM_INFO_H

#include <vulkan/vulkan.h>
#include <vector>

namespace lvk {

class SystemInfo {
public:
    // Use get_system_info to create a SystemInfo struct. This is because loading vulkan could fail.
    static SystemInfo get_system_info();

    bool IsLayerAvailable(const char *layer_name) const;

    bool IsExtensionAvailable(const char *extension_name) const;

    bool IsInstanceVersionAvailable(uint32_t major_api_version, uint32_t minor_api_version) const;

    bool IsInstanceVersionAvailable(uint32_t api_version) const;

    bool CheckLayerSupported(const char *layer_name) const;

    bool CheckLayersSupported(std::vector<const char *> const &layer_names) const;

    bool CheckExtensionSupported(const char *extension_name) const;
    bool CheckExtensionsSupported(std::vector<const char *> const &extensions) const;

    std::vector<VkLayerProperties> available_layers;
    std::vector<VkExtensionProperties> available_extensions;
    bool validation_layers_available = false;
    bool debug_utils_available = false;

    uint32_t instance_api_version = VK_API_VERSION_1_0;
    const char* validation_layer_name = "VK_LAYER_KHRONOS_validation";

private:
    SystemInfo();

};

} // end namespace lvk

#endif //LVK_SYSTEM_INFO_H
