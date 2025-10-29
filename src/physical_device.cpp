//
// Created by lyhux on 2025/10/27.
//

#include "physical_device.h"
#include "functions.h"

#include <algorithm>
#include <stdexcept>

namespace lvk {
PhysicalDevice PhysicalDeviceSelector::PopulateDeviceDetails(VkPhysicalDevice vk_phys_device,
                                                             FeatureChain const &src_extended_features_chain)
const {
    PhysicalDevice physical_device{};
    physical_device.physical_device = vk_phys_device;
    physical_device.surface = instance_info_.surface;
    physical_device.defer_surface_initialization_ = criteria_.defer_surface_initialization;
    physical_device.instance_version_ = instance_info_.version;

    auto queue_families = get_vector_noerror<VkQueueFamilyProperties>(
            vkGetPhysicalDeviceQueueFamilyProperties, vk_phys_device);

    physical_device.queue_families_ = queue_families;

    vkGetPhysicalDeviceProperties(vk_phys_device, &physical_device.properties);
    vkGetPhysicalDeviceFeatures(vk_phys_device, &physical_device.features);
    vkGetPhysicalDeviceMemoryProperties(vk_phys_device, &physical_device.memory_properties);

    physical_device.name = physical_device.properties.deviceName;

    std::vector<VkExtensionProperties> available_extensions;
    auto available_extensions_ret = get_vector<VkExtensionProperties>(
            available_extensions, vkEnumerateDeviceExtensionProperties, vk_phys_device, nullptr);
    if (available_extensions_ret != VK_SUCCESS) return physical_device;
    for (const auto &ext: available_extensions) {
        physical_device.available_extensions_.emplace_back(&ext.extensionName[0]);
    }
    // Lets us quickly find extensions as this list can be 300+ elements long
    std::sort(physical_device.available_extensions_.begin(), physical_device.available_extensions_.end());

    physical_device.properties2_ext_enabled_ = instance_info_.properties2_ext_enabled;

    auto fill_chain = src_extended_features_chain;

    bool instance_is_1_1 = instance_info_.version >= VK_API_VERSION_1_1;
    if (!fill_chain.Empty() && (instance_is_1_1 || instance_info_.properties2_ext_enabled)) {
        VkPhysicalDeviceFeatures2 local_features{};
        fill_chain.CreateChainedFeatures(local_features);
        // Use KHR function if not able to use the core function
        if (instance_is_1_1) {
            vkGetPhysicalDeviceFeatures2(vk_phys_device, &local_features);
        } else {
            //
            auto func =
                    get_instance_proc_addr<PFN_vkGetPhysicalDeviceFeatures2KHR>(
                            instance_info_.instance, "vkGetPhysicalDeviceFeatures2KHR");
            func(vk_phys_device, &local_features);
        }
        physical_device.extended_features_chain_ = std::move(fill_chain);
    }

    return physical_device;
}

PhysicalDevice::Suitable PhysicalDeviceSelector::IsDeviceSuitable(
        PhysicalDevice const &pd, std::vector<std::string> &unsuitability_reasons) const {
    PhysicalDevice::Suitable suitable = PhysicalDevice::Suitable::yes;

    if (!criteria_.name.empty() && criteria_.name != pd.properties.deviceName) {
        unsuitability_reasons.push_back(
                "VkPhysicalDeviceProperties::deviceName doesn't match requested name \"" + criteria_.name + "\"");
        return PhysicalDevice::Suitable::no;
    }

    if (criteria_.required_version > pd.properties.apiVersion) {
        unsuitability_reasons.push_back(
                "VkPhysicalDeviceProperties::apiVersion " + std::to_string(
                        VK_API_VERSION_MAJOR(pd.properties.apiVersion)) +
                "." + std::to_string(VK_API_VERSION_MINOR(pd.properties.apiVersion)) + " lower than required version " +
                std::to_string(VK_API_VERSION_MAJOR(criteria_.required_version)) + "." +
                std::to_string(VK_API_VERSION_MINOR(criteria_.required_version)));
        return PhysicalDevice::Suitable::no;
    }

    bool dedicated_compute = get_dedicated_queue_index(pd.queue_families_, VK_QUEUE_COMPUTE_BIT,
                                                       VK_QUEUE_TRANSFER_BIT) != QUEUE_INDEX_MAX_VALUE;
    bool dedicated_transfer = get_dedicated_queue_index(pd.queue_families_, VK_QUEUE_TRANSFER_BIT,
                                                        VK_QUEUE_COMPUTE_BIT) != QUEUE_INDEX_MAX_VALUE;
    bool separate_compute = get_separate_queue_index(pd.queue_families_, VK_QUEUE_COMPUTE_BIT,
                                                     VK_QUEUE_TRANSFER_BIT) != QUEUE_INDEX_MAX_VALUE;
    bool separate_transfer = get_separate_queue_index(pd.queue_families_, VK_QUEUE_TRANSFER_BIT,
                                                      VK_QUEUE_COMPUTE_BIT) != QUEUE_INDEX_MAX_VALUE;

    bool present_queue = get_present_queue_index(pd.physical_device, instance_info_.surface, pd.queue_families_) !=
                         QUEUE_INDEX_MAX_VALUE;

    if (criteria_.require_dedicated_compute_queue && !dedicated_compute) {
        unsuitability_reasons.emplace_back("No dedicated compute queue");
        return PhysicalDevice::Suitable::no;
    }
    if (criteria_.require_dedicated_transfer_queue && !dedicated_transfer) {
        unsuitability_reasons.emplace_back("No dedicated transfer queue");
        return PhysicalDevice::Suitable::no;
    }
    if (criteria_.require_separate_compute_queue && !separate_compute) {
        unsuitability_reasons.emplace_back("No separate compute queue");
        return PhysicalDevice::Suitable::no;
    }
    if (criteria_.require_separate_transfer_queue && !separate_transfer) {
        unsuitability_reasons.emplace_back("No separate transfer queue");
        return PhysicalDevice::Suitable::no;
    }
    if (criteria_.require_present && !present_queue && !criteria_.defer_surface_initialization) {
        unsuitability_reasons.emplace_back("No queue capable of present operations");
        return PhysicalDevice::Suitable::no;
    }
    const auto unsupported_extensions = find_unsupported_extensions_in_list(
            pd.available_extensions_, criteria_.required_extensions);
    if (!unsupported_extensions.empty()) {
        for (auto const &unsupported_ext: unsupported_extensions) {
            unsuitability_reasons.push_back("Device extension " + unsupported_ext + " not supported");
        }
        return PhysicalDevice::Suitable::no;
    }
    if (!criteria_.defer_surface_initialization && criteria_.require_present) {
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;

        auto formats_ret = get_vector<VkSurfaceFormatKHR>(formats, vkGetPhysicalDeviceSurfaceFormatsKHR,
                                                          pd.physical_device, instance_info_.surface);
        auto present_modes_ret = get_vector<VkPresentModeKHR>(present_modes,
                                                              vkGetPhysicalDeviceSurfacePresentModesKHR,
                                                              pd.physical_device,
                                                              instance_info_.surface);

        if (formats_ret != VK_SUCCESS || present_modes_ret != VK_SUCCESS || formats.empty() || present_modes.
                empty()) {
            if (formats_ret != VK_SUCCESS) {
                unsuitability_reasons.emplace_back(
                        "vkGetPhysicalDeviceSurfaceFormatsKHR returned error code " + std::to_string(formats_ret));
            }
            if (present_modes_ret != VK_SUCCESS) {
                unsuitability_reasons.emplace_back(
                        "vkGetPhysicalDeviceSurfacePresentModesKHR returned error code " + std::to_string(
                                present_modes_ret));
            }
            if (formats.empty()) {
                unsuitability_reasons.emplace_back(
                        "vkGetPhysicalDeviceSurfaceFormatsKHR returned zero surface formats");
            }
            if (present_modes.empty()) {
                unsuitability_reasons.emplace_back(
                        "vkGetPhysicalDeviceSurfacePresentModesKHR returned zero present modes");
            }
            return PhysicalDevice::Suitable::no;
        }
    }

    if (pd.properties.deviceType != static_cast<VkPhysicalDeviceType>(criteria_.preferred_type)) {
        if (criteria_.allow_any_type) {
            suitable = PhysicalDevice::Suitable::partial;
        } else {
            suitable = PhysicalDevice::Suitable::no;
        }
    }


    for (uint32_t i = 0; i < pd.memory_properties.memoryHeapCount; i++) {
        if (pd.memory_properties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            if (pd.memory_properties.memoryHeaps[i].size < criteria_.required_mem_size) {
                unsuitability_reasons.emplace_back("Did not contain a Device Local memory heap with enough size");
                return PhysicalDevice::Suitable::no;
            }
        }
    }

    return suitable;
}

// delegate construction to the one with an explicit surface parameter
PhysicalDeviceSelector::PhysicalDeviceSelector(Instance const &instance)
        : PhysicalDeviceSelector(instance, VK_NULL_HANDLE) {
}

PhysicalDeviceSelector::PhysicalDeviceSelector(Instance const &instance, VkSurfaceKHR surface) {
    instance_info_.instance = instance.instance;
    instance_info_.version = instance.instance_version;
    instance_info_.properties2_ext_enabled = instance.properties2_ext_enabled;
    instance_info_.surface = surface;
    criteria_.require_present = !instance.headless;
    criteria_.required_version = instance.api_version;
}

// Return all devices which are considered suitable - intended for applications which want to let the user pick the physical device
std::vector<PhysicalDevice> PhysicalDeviceSelector::SelectDevices() const {
    std::vector<PhysicalDevice> empty_device{};

    if (criteria_.require_present && !criteria_.defer_surface_initialization) {
        if (instance_info_.surface == VK_NULL_HANDLE)
            return empty_device;
    }

    // Get the VkPhysicalDevice handles on the system
    std::vector<VkPhysicalDevice> vk_physical_devices;

    auto vk_physical_devices_ret = get_vector<VkPhysicalDevice>(vk_physical_devices, vkEnumeratePhysicalDevices,
                                                                instance_info_.instance);
    if (vk_physical_devices_ret != VK_SUCCESS) {
        return empty_device;
    }
    if (vk_physical_devices.empty()) {
        return empty_device;
    }

    auto fill_out_phys_dev_with_criteria = [&](PhysicalDevice &phys_dev) {
        phys_dev.features = criteria_.required_features;
        phys_dev.extended_features_chain_ = criteria_.extended_features_chain;

        bool portability_ext_available =
                criteria_.enable_portability_subset &&
                std::binary_search(phys_dev.available_extensions_.begin(), phys_dev.available_extensions_.end(),
                                   "VK_KHR_portability_subset");

        phys_dev.extensions_to_enable_.clear();
        phys_dev.extensions_to_enable_.insert(
                phys_dev.extensions_to_enable_.end(), criteria_.required_extensions.begin(),
                criteria_.required_extensions.end());
        if (portability_ext_available) {
            phys_dev.extensions_to_enable_.emplace_back("VK_KHR_portability_subset");
        }
        // Lets us quickly find extensions as this list can be 300+ elements long
        std::sort(phys_dev.extensions_to_enable_.begin(), phys_dev.extensions_to_enable_.end());
    };

    // if this option is set, always return only the first physical device found
    if (criteria_.use_first_gpu_unconditionally && !vk_physical_devices.empty()) {
        PhysicalDevice physical_device = PopulateDeviceDetails(vk_physical_devices[0],
                                                               criteria_.extended_features_chain);
        fill_out_phys_dev_with_criteria(physical_device);
        return std::vector<PhysicalDevice>{physical_device};
    }

    // Populate their details and check their suitability
    std::vector<std::string> unsuitability_reasons;
    std::vector<PhysicalDevice> physical_devices;
    for (auto &vk_physical_device: vk_physical_devices) {
        PhysicalDevice phys_dev = PopulateDeviceDetails(vk_physical_device, criteria_.extended_features_chain);
        std::vector<std::string> gpu_unsuitability_reasons;
        phys_dev.suitable = IsDeviceSuitable(phys_dev, gpu_unsuitability_reasons);
        if (phys_dev.suitable != PhysicalDevice::Suitable::no) {
            physical_devices.push_back(phys_dev);
        } else {
            for (auto const &reason: gpu_unsuitability_reasons) {
                unsuitability_reasons.push_back(
                        std::string("Physical Device ") + phys_dev.properties.deviceName + " not selected due to: " +
                        reason);
            }
        }
    }

    // No suitable devices found, return an error which contains the list of reason why it wasn't suitable
    if (physical_devices.empty()) {
        return empty_device;
    }

    // sort the list into fully and partially suitable devices. use stable_partition to maintain relative order
    std::stable_partition(physical_devices.begin(), physical_devices.end(), [](auto const &pd) {
        return pd.suitable == PhysicalDevice::Suitable::yes;
    });

    // Make the physical device ready to be used to create a Device from it
    for (auto &physical_device: physical_devices) {
        fill_out_phys_dev_with_criteria(physical_device);
    }

    return physical_devices;
}

PhysicalDevice PhysicalDeviceSelector::Select() const {
    auto const selected_devices = SelectDevices();

    if (selected_devices.empty()) {
        throw std::runtime_error("without select devices");
    }
    return selected_devices.at(0);
}

std::vector<std::string> PhysicalDeviceSelector::SelectDeviceNames() const {
    auto const selected_devices = SelectDevices();
    if (selected_devices.empty()) return {};

    std::vector<std::string> names;
    for (const auto &pd: selected_devices) {
        names.push_back(pd.name);
    }
    return names;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::SetSurface(VkSurfaceKHR surface) {
    instance_info_.surface = surface;
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::SetName(std::string const &name) {
    criteria_.name = name;
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::PreferGpuDeviceType(PreferredDeviceType type) {
    criteria_.preferred_type = type;
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::AllowAnyGpuDeviceType(bool allow_any_type) {
    criteria_.allow_any_type = allow_any_type;
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::RequirePresent(bool require) {
    criteria_.require_present = require;
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::RequireDedicatedTransferQueue() {
    criteria_.require_dedicated_transfer_queue = true;
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::RequireDedicatedComputeQueue() {
    criteria_.require_dedicated_compute_queue = true;
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::RequireSeparateTransferQueue() {
    criteria_.require_separate_transfer_queue = true;
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::RequireSeparateComputeQueue() {
    criteria_.require_separate_compute_queue = true;
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::RequiredDeviceMemorySize(VkDeviceSize size) {
    criteria_.required_mem_size = size;
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::AddRequiredExtension(const char *extension) {
    criteria_.required_extensions.emplace_back(extension);
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::AddRequiredExtensions(size_t count, const char *const *extensions) {
    if (!extensions || count == 0) return *this;
    for (size_t i = 0; i < count; i++) {
        criteria_.required_extensions.emplace_back(extensions[i]);
    }
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::SetMinimumVersion(uint32_t major, uint32_t minor) {
    criteria_.required_version = VK_MAKE_VERSION(major, minor, 0);
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::DisablePortabilitySubset() {
    criteria_.enable_portability_subset = false;
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::SetRequiredFeatures(VkPhysicalDeviceFeatures const &features) {
    // VkPhysicalDeviceFeatures(criteria_.required_features, features);
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::DeferSurfaceInitialization() {
    criteria_.defer_surface_initialization = true;
    return *this;
}

PhysicalDeviceSelector &PhysicalDeviceSelector::SelectFirstDeviceUnconditionally(bool unconditionally) {
    criteria_.use_first_gpu_unconditionally = unconditionally;
    return *this;
}

// PhysicalDevice
bool PhysicalDevice::HasDedicatedComputeQueue() const {
    return get_dedicated_queue_index(queue_families_, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT) !=
           QUEUE_INDEX_MAX_VALUE;
}

bool PhysicalDevice::HasSeparateComputeQueue() const {
    return get_separate_queue_index(queue_families_, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT) !=
           QUEUE_INDEX_MAX_VALUE;
}

bool PhysicalDevice::HasDedicatedTransferQueue() const {
    return get_dedicated_queue_index(queue_families_, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_COMPUTE_BIT) !=
           QUEUE_INDEX_MAX_VALUE;
}

bool PhysicalDevice::HasSeparateTransferQueue() const {
    return get_separate_queue_index(queue_families_, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_COMPUTE_BIT) !=
           QUEUE_INDEX_MAX_VALUE;
}

std::vector<VkQueueFamilyProperties> PhysicalDevice::GetQueueFamilies() const { return queue_families_; }

std::vector<std::string> PhysicalDevice::GetExtensions() const { return extensions_to_enable_; }

std::vector<std::string> PhysicalDevice::GetAvailableExtensions() const { return available_extensions_; }

bool PhysicalDevice::IsExtensionPresent(const char *ext) const {
    return std::binary_search(std::begin(available_extensions_), std::end(available_extensions_), ext);
}

bool PhysicalDevice::EnableExtensionIfPresent(const char *extension) {
    if (std::binary_search(std::begin(available_extensions_), std::end(available_extensions_), extension)) {
        extensions_to_enable_.insert(
                std::upper_bound(std::begin(extensions_to_enable_), std::end(extensions_to_enable_), extension),
                extension);
        return true;
    }
    return false;
}

bool PhysicalDevice::EnableExtensionsIfPresent(size_t count, const char *const *extensions) {
    for (size_t i = 0; i < count; ++i) {
        const auto extension = extensions[i];
        if (!std::binary_search(std::begin(available_extensions_), std::end(available_extensions_), extension)) {
            return false;
        }
    }

    for (size_t i = 0; i < count; ++i) {
        extensions_to_enable_.insert(
                std::upper_bound(std::begin(extensions_to_enable_), std::end(extensions_to_enable_), extensions[i]),
                extensions[i]);
    }
    return true;
}

bool PhysicalDevice::EnableFeaturesIfPresent(const VkPhysicalDeviceFeatures &features_to_enable) const {
    VkPhysicalDeviceFeatures actual_pdf{};
    vkGetPhysicalDeviceFeatures(physical_device, &actual_pdf);

    std::vector<std::string> unsupported_features;
    // detail::compare_VkPhysicalDeviceFeatures(unsupported_features, actual_pdf, features_to_enable);
    // if (unsupported_features.empty()) {
    //     detail::merge_VkPhysicalDeviceFeatures(features, features_to_enable);
    //     return true;
    // }
    return false;
}

bool PhysicalDevice::EnableFeaturesStructIfPresent(
        VkStructureType sType, size_t struct_size, const void *features_struct, void *query_struct) {
    VkPhysicalDeviceFeatures2 actual_pdf2{};
    actual_pdf2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    actual_pdf2.pNext = query_struct;

    // bool instance_is_1_1 = instance_version >= VKB_VK_API_VERSION_1_1;
    // if (instance_is_1_1 || properties2_ext_enabled) {
    //     if (instance_is_1_1) {
    //         detail::vulkan_functions().fp_vkGetPhysicalDeviceFeatures2(physical_device, &actual_pdf2);
    //     } else {
    //         detail::vulkan_functions().fp_vkGetPhysicalDeviceFeatures2KHR(physical_device, &actual_pdf2);
    //     }
    //
    //     std::vector<std::string> error_list;
    //     detail::compare_feature_struct(sType, error_list, query_struct, features_struct);
    //
    //     if (error_list.empty()) {
    //         extended_features_chain.add_structure(sType, struct_size, features_struct);
    //         return true;
    //     }
    // }
    return false;
}


PhysicalDevice::operator VkPhysicalDevice() const { return this->physical_device; }

// ---- Queues ---- //

uint32_t Device::GetQueueIndex(QueueType type) const {
    uint32_t index = QUEUE_INDEX_MAX_VALUE;
    switch (type) {
        case QueueType::kPresent:
            index = get_present_queue_index(physical_device.physical_device, surface, queue_families);
            if (index == QUEUE_INDEX_MAX_VALUE) {
                throw std::runtime_error("failed to present unavailable");
            }
            break;
        case QueueType::kGraphics:
            index = get_first_queue_index(queue_families, VK_QUEUE_GRAPHICS_BIT);
            if (index == QUEUE_INDEX_MAX_VALUE) {
                throw std::runtime_error("failed to graphics unavailable");
            }
            break;
        case QueueType::kCompute:
            index = get_separate_queue_index(queue_families, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT);
            if (index == QUEUE_INDEX_MAX_VALUE) {
                throw std::runtime_error("failed to compute unavailable");
            }
            break;
        case QueueType::kTransfer:
            index = get_separate_queue_index(queue_families, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_COMPUTE_BIT);
            if (index == QUEUE_INDEX_MAX_VALUE) {
                throw std::runtime_error("failed to transfer unavailable");
            }
            break;
        default:
            throw std::runtime_error("invalid_queue_family_index");
    }

    return index;
}

uint32_t Device::GetDedicatedQueueIndex(QueueType type) const {
    uint32_t index = QUEUE_INDEX_MAX_VALUE;
    switch (type) {
        case QueueType::kCompute:
            index = get_dedicated_queue_index(queue_families, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT);
            if (index == QUEUE_INDEX_MAX_VALUE) {
                throw std::runtime_error("failed to compute unavailable");
            }
            break;
        case QueueType::kTransfer:
            index = get_dedicated_queue_index(queue_families, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_COMPUTE_BIT);
            if (index == QUEUE_INDEX_MAX_VALUE) {

                throw std::runtime_error("failed to transfer unavailable");
            }
            break;
        default:
            throw std::runtime_error("invalid_queue_family_index");
    }

    return index;
}

VkQueue Device::GetQueue(QueueType type) const {
    auto index = GetQueueIndex(type);
    VkQueue out_queue;
    internal_table.fp_vkGetDeviceQueue(device, index, 0, &out_queue);
    return out_queue;
}

VkQueue Device::GetDedicatedQueue(QueueType type) const {
    auto index = GetDedicatedQueueIndex(type);

    VkQueue out_queue;
    vkGetDeviceQueue(device, index, 0, &out_queue);
    return out_queue;
}

// ---- Device ---- //
Device::operator VkDevice() const { return this->device; }

void destroy_device(Device const &device) {
    if (device.device != VK_NULL_HANDLE) {
        device.internal_table.fp_vkDestroyDevice(device.device, device.allocation_callbacks);
    }
}

DeviceBuilder::DeviceBuilder(PhysicalDevice phys_device) { physical_device = std::move(phys_device); }

Device DeviceBuilder::Build() const {
    std::vector<CustomQueueDescription> queue_descriptions;
    queue_descriptions.insert(queue_descriptions.end(), info.queue_descriptions.begin(),
                              info.queue_descriptions.end());

    if (queue_descriptions.empty()) {
        for (uint32_t i = 0; i < physical_device.queue_families_.size(); i++) {
            queue_descriptions.emplace_back(i, std::vector<float>{1.0f});
        }
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (auto &desc: queue_descriptions) {
        VkDeviceQueueCreateInfo queue_create_info = {};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = desc.index;
        queue_create_info.queueCount = static_cast<std::uint32_t>(desc.priorities.size());
        queue_create_info.pQueuePriorities = desc.priorities.data();
        queueCreateInfos.push_back(queue_create_info);
    }

    std::vector<const char *> extensions_to_enable;
    for (const auto &ext: physical_device.extensions_to_enable_) {
        extensions_to_enable.push_back(ext.c_str());
    }
    if (physical_device.surface != VK_NULL_HANDLE || physical_device.defer_surface_initialization_)
        extensions_to_enable.push_back({VK_KHR_SWAPCHAIN_EXTENSION_NAME});

    std::vector<void *> final_pnext_chain;
    VkDeviceCreateInfo device_create_info = {};

    bool user_defined_phys_dev_features_2 = false;
    for (auto &next: info.next_chain) {
        VkBaseOutStructure out_structure{};
        memcpy(&out_structure, next, sizeof(VkBaseOutStructure));
        if (out_structure.sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2) {
            user_defined_phys_dev_features_2 = true;
            break;
        }
    }

    if (user_defined_phys_dev_features_2 && !physical_device.extended_features_chain_.Empty()) {
        throw std::runtime_error(
                "VkPhysicalDeviceFeatures2_in_pNext_chain_while_using_add_required_extension_features");
    }

    // These objects must be alive during the call to vkCreateDevice
    auto physical_device_extension_features_copy = physical_device.extended_features_chain_;
    VkPhysicalDeviceFeatures2 local_features2{};
    local_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    local_features2.features = physical_device.features;


    for (auto &next: info.next_chain) {
        final_pnext_chain.push_back(next);
    }

    setup_next_chain(device_create_info, final_pnext_chain);

    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.flags = info.flags;
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    device_create_info.pQueueCreateInfos = queueCreateInfos.data();
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions_to_enable.size());
    device_create_info.ppEnabledExtensionNames = extensions_to_enable.data();

    Device device;

    VkResult res = vkCreateDevice(
            physical_device.physical_device, &device_create_info, info.allocation_callbacks, &device.device);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("failed_create_device");
    }

    device.physical_device = physical_device;
    device.surface = physical_device.surface;
    device.queue_families = physical_device.queue_families_;
    device.allocation_callbacks = info.allocation_callbacks;
    device.fp_vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    device.internal_table.fp_vkGetDeviceQueue = vkGetDeviceQueue;
    // get_device_proc_addr(device.device, device.internal_table.fp_vkGetDeviceQueue,
    //                                                 "vkGetDeviceQueue");
    device.internal_table.fp_vkDestroyDevice = vkDestroyDevice;
    // detail::vulkan_functions().get_device_proc_addr(device.device, device.internal_table.fp_vkDestroyDevice,
    //                                                 "vkDestroyDevice");
    device.instance_version = physical_device.instance_version_;
    return device;
}

DeviceBuilder &DeviceBuilder::SetAllocationCallbacks(VkAllocationCallbacks *callbacks) {
    info.allocation_callbacks = callbacks;
    return *this;
}

DeviceBuilder &DeviceBuilder::CustomQueueSetup(size_t count, CustomQueueDescription const *queue_descriptions) {
    info.queue_descriptions.assign(queue_descriptions, queue_descriptions + count);
    return *this;
}

DeviceBuilder &DeviceBuilder::CustomQueueSetup(std::vector<CustomQueueDescription> const &queue_descriptions) {
    info.queue_descriptions = queue_descriptions;
    return *this;
}

DeviceBuilder &DeviceBuilder::CustomQueueSetup(std::vector<CustomQueueDescription> &&queue_descriptions) {
    info.queue_descriptions = std::move(queue_descriptions);
    return *this;
}

} // end namespace lvk
