//
// Created by admin on 2025/10/27.
//

#ifndef LVK_INSTANCE_H
#define LVK_INSTANCE_H
#include <vulkan/vulkan.h>
#include <vector>

#include "functions.h"

namespace lvk {


class InstanceBuilder;

class Instance {
public:
    ~Instance();

private:
    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
    VkAllocationCallbacks* allocation_callbacks_ = nullptr;
    PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr_ = nullptr;
    PFN_vkGetDeviceProcAddr fp_vkGetDeviceProcAddr_ = nullptr;
    // The apiVersion used to create the instance
    uint32_t instance_version_ = VK_API_VERSION_1_1;
    // The instance version queried from vkEnumerateInstanceVersion
    uint32_t api_version_ = VK_API_VERSION_1_1;


    // A conversion function which allows this Instance to be used
    // in places where VkInstance would have been used.
    explicit operator VkInstance() const;


private:
    bool headless_ = false;
    bool properties2_ext_enabled_ = false;

    friend class InstanceBuilder;
};



class InstanceBuilder {
public:
    // Default constructor, will load vulkan.
    explicit InstanceBuilder();
    // Optional: Can use your own PFN_vkGetInstanceProcAddr
    // explicit InstanceBuilder(PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr);

    // Create a VkInstance. Return an error if it failed.
    Instance build() const;

    // Sets the name of the application. Defaults to "" if none is provided.
    InstanceBuilder& SetAppName(const char* app_name);
    // Sets the name of the engine. Defaults to "" if none is provided.
    InstanceBuilder& SetEngineName(const char* engine_name);

    // Sets the version of the application.
    // Should be constructed with VK_MAKE_VERSION or VK_MAKE_API_VERSION.
    InstanceBuilder& SetAppVersion(uint32_t app_version);
    // Sets the (major, minor, patch) version of the application.
    InstanceBuilder& SetAppVersion(uint32_t major, uint32_t minor, uint32_t patch = 0);

    // Sets the version of the engine.
    // Should be constructed with VK_MAKE_VERSION or VK_MAKE_API_VERSION.
    InstanceBuilder& SetEngineVersion(uint32_t engine_version);
    // Sets the (major, minor, patch) version of the engine.
    InstanceBuilder& SetEngineVersion(uint32_t major, uint32_t minor, uint32_t patch = 0);

    // Require a vulkan API version. Will fail to create if this version isn't available.
    // Should be constructed with VK_MAKE_VERSION or VK_MAKE_API_VERSION.
    InstanceBuilder& RequireApiVersion(uint32_t required_api_version);
    // Require a vulkan API version. Will fail to create if this version isn't available.
    InstanceBuilder& RequireApiVersion(uint32_t major, uint32_t minor, uint32_t patch = 0);

    // Overrides required API version for instance creation. Will fail to create if this version isn't available.
    // Should be constructed with VK_MAKE_VERSION or VK_MAKE_API_VERSION.
    InstanceBuilder& SetMinimumInstanceVersion(uint32_t minimum_instance_version);
    // Overrides required API version for instance creation. Will fail to create if this version isn't available.
    InstanceBuilder& SetMinimumInstanceVersion(uint32_t major, uint32_t minor, uint32_t patch = 0);

    // Adds a layer to be enabled. Will fail to create an instance if the layer isn't available.
    InstanceBuilder& EnableLayer(const char* layer_name);
    // Adds an extension to be enabled. Will fail to create an instance if the extension isn't available.
    InstanceBuilder& EnableExtension(const char* extension_name);

    // Add extensions to be enabled. Will fail to create an instance if the extension aren't available.
    InstanceBuilder& EnableExtensions(size_t count, const char* const* extensions);

    // Add extensions to be enabled. Will fail to create an instance if the extension aren't available.
    InstanceBuilder& EnableExtensions(std::vector<const char*> const& extensions) {
        return EnableExtensions(extensions.size(), extensions.data());
    }

    // Headless Mode does not load the required extensions for presentation. Defaults to true.
    InstanceBuilder& SetHeadless(bool headless = true);

    // Enables the validation layers. Will fail to create an instance if the validation layers aren't available.
    InstanceBuilder& EnableValidationLayers(bool require_validation = true);
    // Checks if the validation layers are available and loads them if they are.
    InstanceBuilder& RequestValidationLayers(bool enable_validation = true);

    // Use a default debug callback that prints to standard out.
    InstanceBuilder& UseDefaultDebugMessenger();
    // Provide a user defined debug callback.
    InstanceBuilder& SetDebugCallback(PFN_vkDebugUtilsMessengerCallbackEXT callback);
    // Sets the void* to use in the debug messenger - only useful with a custom callback
    InstanceBuilder& SetDebugCallbackUserDataPointer(void* user_data_pointer);
    // Set what message severity is needed to trigger the callback.
    InstanceBuilder& SetDebugMessengerSeverity(VkDebugUtilsMessageSeverityFlagsEXT severity);
    // Add a message severity to the list that triggers the callback.
    InstanceBuilder& AddDebugMessengerSeverity(VkDebugUtilsMessageSeverityFlagsEXT severity);
    // Set what message type triggers the callback.
    InstanceBuilder& SetDebugMessengerType(VkDebugUtilsMessageTypeFlagsEXT type);
    // Add a message type to the list of that triggers the callback.
    InstanceBuilder& AddDebugMessengerType(VkDebugUtilsMessageTypeFlagsEXT type);

    // Disable some validation checks.
    // Checks: All, and Shaders
    InstanceBuilder& AddValidationDisable(VkValidationCheckEXT check);

    // Enables optional parts of the validation layers.
    // Parts: best practices, gpu assisted, and gpu assisted reserve binding slot.
    InstanceBuilder& AddValidationFeatureEnable(VkValidationFeatureEnableEXT enable);

    // Disables sections of the validation layers.
    // Options: All, shaders, thread safety, api parameters, object lifetimes, core checks, and unique handles.
    InstanceBuilder& AddValidationFeatureDisable(VkValidationFeatureDisableEXT disable);

    // Provide custom allocation callbacks.
    InstanceBuilder& SetAllocationCallbacks(VkAllocationCallbacks* callbacks);

    // Set a setting on a requested layer via VK_EXT_layer_settings
    InstanceBuilder& AddLayerSetting(VkLayerSettingEXT setting);

private:
    struct InstanceInfo {
        // VkApplicationInfo
        const char* app_name = nullptr;
        const char* engine_name = nullptr;
        uint32_t application_version = 0;
        uint32_t engine_version = 0;
        uint32_t minimum_instance_version = 0;
        uint32_t required_api_version = VK_API_VERSION_1_0;

        // VkInstanceCreateInfo
        std::vector<const char*> layers;
        std::vector<const char*> extensions;
        VkInstanceCreateFlags flags = static_cast<VkInstanceCreateFlags>(0);
        std::vector<VkLayerSettingEXT> layer_settings;

        // debug callback - use the default so it is not nullptr
        PFN_vkDebugUtilsMessengerCallbackEXT debug_callback = default_debug_callback;
        VkDebugUtilsMessageSeverityFlagsEXT debug_message_severity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        VkDebugUtilsMessageTypeFlagsEXT debug_message_type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        void* debug_user_data_pointer = nullptr;

        // validation features
        std::vector<VkValidationCheckEXT> disabled_validation_checks;
        std::vector<VkValidationFeatureEnableEXT> enabled_validation_features;
        std::vector<VkValidationFeatureDisableEXT> disabled_validation_features;

        // Custom allocator
        VkAllocationCallbacks* allocation_callbacks = nullptr;

        bool request_validation_layers = false;
        bool enable_validation_layers = false;
        bool use_debug_messenger = false;
        bool headless_context = false;

        PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr = nullptr;
    } info;
};


} // end namespace lvk

#endif //LVK_INSTANCE_H
