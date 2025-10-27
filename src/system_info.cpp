//
// Created by admin on 2025/10/27.
//

#include "system_info.h"

namespace lvk {

SystemInfo SystemInfo::get_system_info() {
    return {};
}


SystemInfo::SystemInfo() {
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);

    if (count > 0) {
        available_layers.resize(count);
        vkEnumerateInstanceLayerProperties(&count, available_layers.data());

        for (auto &layer: this->available_layers) {
            if (strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0) validation_layers_available = true;

            count = 0;
            vkEnumerateInstanceExtensionProperties(layer.layerName, &count, nullptr);

            if (count > 0) {
                std::vector<VkExtensionProperties> layer_extensions(count);
                vkEnumerateInstanceExtensionProperties(layer.layerName, &count, layer_extensions.data());
                // add all layer extensions
                this->available_extensions.insert(
                        this->available_extensions.end(), layer_extensions.begin(), layer_extensions.end());

                for (auto &ext: layer_extensions) {
                    if (strcmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
                        debug_utils_available = true;
                    }
                }
            }
        }
    }


    vkEnumerateInstanceVersion(&instance_api_version);

}

bool SystemInfo::IsExtensionAvailable(const char *extension_name) const {
    if (!extension_name) return false;
    return CheckExtensionSupported(extension_name);
}

bool SystemInfo::IsLayerAvailable(const char *layer_name) const {
    if (!layer_name) return false;
    return CheckLayerSupported(layer_name);
}

bool SystemInfo::IsInstanceVersionAvailable(uint32_t major_api_version, uint32_t minor_api_version) const {
    return instance_api_version >= VK_MAKE_VERSION(major_api_version, minor_api_version, 0);
}

bool SystemInfo::CheckLayersSupported(std::vector<const char *> const &layer_names) const {
    bool all_found = true;
    for (const auto &layer_name: layer_names) {
        bool found = CheckLayerSupported(layer_name);
        if (!found) all_found = false;
    }
    return all_found;
}

bool SystemInfo::CheckLayerSupported(const char *layer_name) const {
    if (!layer_name) return false;
    for (const auto &layer_properties: available_layers) {
        if (strcmp(layer_name, layer_properties.layerName) == 0) {
            return true;
        }
    }
    return false;
}

bool SystemInfo::CheckExtensionSupported(const char *extension_name) const {
    if (!extension_name) return false;
    for (const auto &extension_properties: available_extensions) {
        if (strcmp(extension_name, extension_properties.extensionName) == 0) {
            return true;
        }
    }

    return false;
}

bool SystemInfo::IsInstanceVersionAvailable(uint32_t api_version) const {
    return instance_api_version >= api_version;
}

bool SystemInfo::CheckExtensionsSupported(const std::vector<const char *> &extensions) const {
    bool all_found = true;
    for (const auto &extension: available_extensions) {
        bool found = CheckExtensionSupported(extension.extensionName);
        if (!found) all_found = false;
    }

    return all_found;
}


} // end namespace lvk
