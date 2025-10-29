//
// Created by admin on 2025/10/27.
//

#include <stdexcept>
#include "instance.h"
#include "system_info.h"

namespace lvk {

Instance::operator VkInstance() const {
    return instance;
}

void destroy_instance(Instance const& instance) {
    if (instance.instance != VK_NULL_HANDLE) {
        if (instance.debug_messenger != VK_NULL_HANDLE)
            destroy_debug_utils_messenger(instance.instance, instance.debug_messenger, instance.allocation_callbacks);
        vkDestroyInstance(instance.instance, instance.allocation_callbacks);
    }
}
void destroy_surface(Instance const& instance, VkSurfaceKHR surface) {
    if (instance.instance != VK_NULL_HANDLE && surface != VK_NULL_HANDLE) {
        auto pVkDestroySurfaceKHR = get_instance_proc_addr<PFN_vkDestroySurfaceKHR>(instance.instance, "vkDestroySurfaceKHR");
        pVkDestroySurfaceKHR(instance.instance, surface, instance.allocation_callbacks);
    }
}
void destroy_surface(VkInstance instance, VkSurfaceKHR surface, VkAllocationCallbacks* callbacks) {
    if (instance != VK_NULL_HANDLE && surface != VK_NULL_HANDLE) {
        auto pVkDestroySurfaceKHR = get_instance_proc_addr<PFN_vkDestroySurfaceKHR>(instance, "vkDestroySurfaceKHR");
        pVkDestroySurfaceKHR(instance, surface, callbacks);
    }
}
// Instance::~Instance() {
//     if (instance_ != VK_NULL_HANDLE) {
//         if (debug_messenger_ != VK_NULL_HANDLE)
//             destroy_debug_utils_messenger(instance_, debug_messenger_, allocation_callbacks_);
//         vkDestroyInstance(instance_, allocation_callbacks_);
//     }
// }

// InstanceBuilder

InstanceBuilder::InstanceBuilder() = default;

Instance InstanceBuilder::Build() const {

    auto system = SystemInfo::get_system_info();


    uint32_t instance_version = VK_API_VERSION_1_0;

    if (info.minimum_instance_version > VK_API_VERSION_1_0 || info.required_api_version > VK_API_VERSION_1_0) {

        instance_version = system.instance_api_version;

        if ((info.minimum_instance_version > 0 && instance_version < info.minimum_instance_version) ||
            (info.minimum_instance_version == 0 && instance_version < info.required_api_version)) {

            uint32_t version_error = info.minimum_instance_version == 0 ? info.required_api_version : info.minimum_instance_version;
            if (VK_VERSION_MINOR(version_error) == 4)
                throw std::runtime_error("vulkan_version_1_4_unavailable");
            else if (VK_VERSION_MINOR(version_error) == 3)
                throw std::runtime_error("vulkan_version_1_3_unavailable");
            else if (VK_VERSION_MINOR(version_error) == 2)
                throw std::runtime_error("vulkan_version_1_2_unavailable");
            else if (VK_VERSION_MINOR(version_error) == 1)
                throw std::runtime_error("vulkan_version_1_1_unavailable");
            else {
                throw std::runtime_error("vulkan_version_unavailable");
            }
        }
    }

    // The API version to use is set by required_api_version, unless it isn't set, then it comes from minimum_instance_version
    uint32_t api_version = VK_API_VERSION_1_0;
    if (info.required_api_version > VK_API_VERSION_1_0) {
        api_version = info.required_api_version;
    } else if (info.minimum_instance_version > 0) {
        api_version = info.minimum_instance_version;
    }

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = info.app_name != nullptr ? info.app_name : "";
    app_info.applicationVersion = info.application_version;
    app_info.pEngineName = info.engine_name != nullptr ? info.engine_name : "";
    app_info.engineVersion = info.engine_version;
    app_info.apiVersion = api_version;

    std::vector<const char*> extensions;
    std::vector<const char*> layers;

    for (auto& ext : info.extensions)
        extensions.push_back(ext);
    if (info.debug_callback != nullptr && info.use_debug_messenger && system.debug_utils_available) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    bool properties2_ext_enabled =
            api_version < VK_API_VERSION_1_1 && system.CheckExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (properties2_ext_enabled) {
        extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    if (!info.layer_settings.empty()) {
        extensions.push_back(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME);
    }

#if defined(VK_KHR_portability_enumeration)
    bool portability_enumeration_support =
            system.CheckExtensionSupported(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    if (portability_enumeration_support) {
        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    }
#else
    bool portability_enumeration_support = false;
#endif
    if (!info.headless_context) {
        auto check_add_window_ext = [&](const char* name) -> bool {
            if (system.CheckExtensionSupported(name)) {
                extensions.push_back(name);
                return true;
            }

            if (info.available_extensions.empty()) return false;
            for (const auto &extension: info.available_extensions) {
                if (strcmp(name, extension) == 0) {
                    extensions.push_back(name);
                    return true;
                }
            }
            return false;
        };
        bool khr_surface_added = check_add_window_ext("VK_KHR_surface");
#if defined(_WIN32)
        bool added_window_ext = check_add_window_ext("VK_KHR_win32_surface");
#elif defined(__ANDROID__)
        bool added_window_ext = check_add_window_ext("VK_KHR_android_surface");
#elif defined(_DIRECT2DISPLAY)
        bool added_window_ext = check_add_window_ext("VK_KHR_display");
#elif defined(__linux__) || defined(__FreeBSD__)
        // make sure all three calls to check_add_window_ext, don't allow short circuiting
        bool added_window_ext = check_add_window_ext("VK_KHR_xcb_surface");
        added_window_ext = check_add_window_ext("VK_KHR_xlib_surface") || added_window_exts;
        added_window_ext = check_add_window_ext("VK_KHR_wayland_surface") || added_window_exts;
#elif defined(__APPLE__)
        bool added_window_ext = check_add_window_ext("VK_EXT_metal_surface");
#endif
        if (!khr_surface_added || !added_window_ext)
            throw std::runtime_error("windowing_extensions_not_present");
    }
    bool all_extensions_supported = system.CheckExtensionsSupported(extensions);
    if (!all_extensions_supported) {
        throw std::runtime_error("requested_extensions_not_present");
    }

    for (auto& layer : info.layers)
        layers.push_back(layer);

    if (info.enable_validation_layers || (info.request_validation_layers && system.validation_layers_available)) {
        layers.push_back(system.validation_layer_name);
    }
    bool all_layers_supported = system.CheckLayersSupported(layers);
    if (!all_layers_supported) {
        throw std::runtime_error("requested_extensions_not_present");
    }

    std::vector<void*>next_chain;

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
    if (info.use_debug_messenger) {
        messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerCreateInfo.pNext = nullptr;
        messengerCreateInfo.messageSeverity = info.debug_message_severity;
        messengerCreateInfo.messageType = info.debug_message_type;
        messengerCreateInfo.pfnUserCallback = info.debug_callback;
        messengerCreateInfo.pUserData = info.debug_user_data_pointer;
        next_chain.push_back(&messengerCreateInfo);
    }

    VkValidationFeaturesEXT features{};
    if (!info.enabled_validation_features.empty() || !info.disabled_validation_features.empty()) {
        features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        features.pNext = nullptr;
        features.enabledValidationFeatureCount = static_cast<uint32_t>(info.enabled_validation_features.size());
        features.pEnabledValidationFeatures = info.enabled_validation_features.data();
        features.disabledValidationFeatureCount = static_cast<uint32_t>(info.disabled_validation_features.size());
        features.pDisabledValidationFeatures = info.disabled_validation_features.data();
        next_chain.push_back(&features);
    }

    VkValidationFlagsEXT checks{};
    if (!info.disabled_validation_checks.empty()) {
        checks.sType = VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT;
        checks.pNext = nullptr;
        checks.disabledValidationCheckCount = static_cast<uint32_t>(info.disabled_validation_checks.size());
        checks.pDisabledValidationChecks = info.disabled_validation_checks.data();
        next_chain.push_back(&checks);
    }

    VkLayerSettingsCreateInfoEXT layer_settings_ci{};
    if (!info.layer_settings.empty()) {
        layer_settings_ci.sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT;
        layer_settings_ci.pNext = nullptr;
        layer_settings_ci.settingCount = static_cast<uint32_t>(info.layer_settings.size());
        layer_settings_ci.pSettings = info.layer_settings.data();
        next_chain.push_back(&layer_settings_ci);
    }

    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    setup_next_chain(instance_create_info, next_chain);

    instance_create_info.flags = info.flags;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instance_create_info.ppEnabledExtensionNames = extensions.data();
    instance_create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
    instance_create_info.ppEnabledLayerNames = layers.data();
#if defined(VK_KHR_portability_enumeration)
    if (portability_enumeration_support) {
        instance_create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

    Instance instance;
    VkResult res = vkCreateInstance(&instance_create_info, info.allocation_callbacks, &instance.instance);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("failed_create_instance");
    }

    if (info.use_debug_messenger) {
        res = create_debug_utils_messenger(instance.instance,
                                           info.debug_callback,
                                           info.debug_message_severity,
                                           info.debug_message_type,
                                           info.debug_user_data_pointer,
                                           &instance.debug_messenger,
                                           info.allocation_callbacks);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("failed_create_debug_messenger");
        }
    }

    instance.headless = info.headless_context;
    instance.properties2_ext_enabled = properties2_ext_enabled;
    instance.allocation_callbacks = info.allocation_callbacks;
    instance.instance_version = instance_version;
    instance.api_version = api_version;
    // instance.fp_vkGetInstanceProcAddr_ = detail::vulkan_functions().ptr_vkGetInstanceProcAddr;
    // instance.fp_vkGetDeviceProcAddr_ = detail::vulkan_functions().fp_vkGetDeviceProcAddr;
    return instance;
}

InstanceBuilder& InstanceBuilder::SetAppName(const char* app_name) {
    if (!app_name) return *this;
    info.app_name = app_name;
    return *this;
}
InstanceBuilder& InstanceBuilder::SetEngineName(const char* engine_name) {
    if (!engine_name) return *this;
    info.engine_name = engine_name;
    return *this;
}
InstanceBuilder& InstanceBuilder::SetAppVersion(uint32_t app_version) {
    info.application_version = app_version;
    return *this;
}
InstanceBuilder& InstanceBuilder::SetAppVersion(uint32_t major, uint32_t minor, uint32_t patch) {
    info.application_version = VK_MAKE_VERSION(major, minor, patch);
    return *this;
}
InstanceBuilder& InstanceBuilder::SetEngineVersion(uint32_t engine_version) {
    info.engine_version = engine_version;
    return *this;
}
InstanceBuilder& InstanceBuilder::SetEngineVersion(uint32_t major, uint32_t minor, uint32_t patch) {
    info.engine_version = VK_MAKE_VERSION(major, minor, patch);
    return *this;
}
InstanceBuilder& InstanceBuilder::RequireApiVersion(uint32_t required_api_version) {
    info.required_api_version = required_api_version;
    return *this;
}
InstanceBuilder& InstanceBuilder::RequireApiVersion(uint32_t major, uint32_t minor, uint32_t patch) {
    info.required_api_version = VK_MAKE_VERSION(major, minor, patch);
    return *this;
}
InstanceBuilder& InstanceBuilder::SetMinimumInstanceVersion(uint32_t minimum_instance_version) {
    info.minimum_instance_version = minimum_instance_version;
    return *this;
}
InstanceBuilder& InstanceBuilder::SetMinimumInstanceVersion(uint32_t major, uint32_t minor, uint32_t patch) {
    info.minimum_instance_version = VK_MAKE_VERSION(major, minor, patch);
    return *this;
}
InstanceBuilder& InstanceBuilder::EnableLayer(const char* layer_name) {
    if (!layer_name) return *this;
    info.layers.push_back(layer_name);
    return *this;
}
InstanceBuilder& InstanceBuilder::EnableExtension(const char* extension_name) {
    if (!extension_name) return *this;
    info.extensions.push_back(extension_name);
    return *this;
}
InstanceBuilder& InstanceBuilder::EnableExtensions(size_t count, const char* const* extensions) {
    if (!extensions || count == 0) return *this;
    for (size_t i = 0; i < count; i++) {
        info.extensions.push_back(extensions[i]);
    }
    return *this;
}
InstanceBuilder& InstanceBuilder::EnableValidationLayers(bool enable_validation) {
    info.enable_validation_layers = enable_validation;
    return *this;
}
InstanceBuilder& InstanceBuilder::RequestValidationLayers(bool enable_validation) {
    info.request_validation_layers = enable_validation;
    return *this;
}

InstanceBuilder& InstanceBuilder::UseDefaultDebugMessenger() {
    info.use_debug_messenger = true;
    info.debug_callback = default_debug_callback;
    return *this;
}
InstanceBuilder& InstanceBuilder::SetDebugCallback(PFN_vkDebugUtilsMessengerCallbackEXT callback) {
    info.use_debug_messenger = true;
    info.debug_callback = callback;
    return *this;
}
InstanceBuilder& InstanceBuilder::SetDebugCallbackUserDataPointer(void* user_data_pointer) {
    info.debug_user_data_pointer = user_data_pointer;
    return *this;
}
InstanceBuilder& InstanceBuilder::SetHeadless(bool headless) {
    info.headless_context = headless;
    return *this;
}
InstanceBuilder& InstanceBuilder::SetDebugMessengerSeverity(VkDebugUtilsMessageSeverityFlagsEXT severity) {
    info.debug_message_severity = severity;
    return *this;
}
InstanceBuilder& InstanceBuilder::AddDebugMessengerSeverity(VkDebugUtilsMessageSeverityFlagsEXT severity) {
    info.debug_message_severity = info.debug_message_severity | severity;
    return *this;
}
InstanceBuilder& InstanceBuilder::SetDebugMessengerType(VkDebugUtilsMessageTypeFlagsEXT type) {
    info.debug_message_type = type;
    return *this;
}
InstanceBuilder& InstanceBuilder::AddDebugMessengerType(VkDebugUtilsMessageTypeFlagsEXT type) {
    info.debug_message_type = info.debug_message_type | type;
    return *this;
}
InstanceBuilder& InstanceBuilder::AddValidationDisable(VkValidationCheckEXT check) {
    info.disabled_validation_checks.push_back(check);
    return *this;
}
InstanceBuilder& InstanceBuilder::AddValidationFeatureEnable(VkValidationFeatureEnableEXT enable) {
    info.enabled_validation_features.push_back(enable);
    return *this;
}
InstanceBuilder& InstanceBuilder::AddValidationFeatureDisable(VkValidationFeatureDisableEXT disable) {
    info.disabled_validation_features.push_back(disable);
    return *this;
}
InstanceBuilder& InstanceBuilder::SetAllocationCallbacks(VkAllocationCallbacks* callbacks) {
    info.allocation_callbacks = callbacks;
    return *this;
}
InstanceBuilder& InstanceBuilder::AddLayerSetting(VkLayerSettingEXT setting) {
    info.layer_settings.push_back(setting);
    return *this;
}

InstanceBuilder &InstanceBuilder::AddAvailableExtensions(size_t count, const char *const *extensions) {
    for (int i = 0; i < count; ++i) {
        info.available_extensions.push_back(extensions[i]);
    }
    return *this;
}


} // end namespace lvk
