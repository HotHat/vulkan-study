//
// Created by lyhux on 2025/10/27.
//

#ifndef LVK_PHYSICAL_DEVICE_H
#define LVK_PHYSICAL_DEVICE_H

#include "feature_chain.h"
#include "instance.h"

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace lvk {
class PhysicalDeviceSelector;

class DeviceBuilder;

struct PhysicalDevice {
    std::string name;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    // Note that this reflects selected features carried over from required
    // features, not all features the physical device supports.
    VkPhysicalDeviceFeatures features{};
    VkPhysicalDeviceProperties properties{};
    VkPhysicalDeviceMemoryProperties memory_properties{};

    // Has a queue family that supports compute operations but not graphics nor
    // transfer.
    bool HasDedicatedComputeQueue() const;

    // Has a queue family that supports transfer operations but not graphics nor
    // compute.
    bool HasDedicatedTransferQueue() const;

    // Has a queue family that supports transfer operations but not graphics.
    bool HasSeparateComputeQueue() const;

    // Has a queue family that supports transfer operations but not graphics.
    bool HasSeparateTransferQueue() const;

    // Advanced: Get the VkQueueFamilyProperties of the device if special queue
    // setup is needed
    std::vector<VkQueueFamilyProperties> GetQueueFamilies() const;

    // Query the list of extensions which should be enabled
    std::vector<std::string> GetExtensions() const;

    // Query the list of extensions which the physical device supports
    std::vector<std::string> GetAvailableExtensions() const;

    // Returns true if an extension should be enabled on the device
    bool IsExtensionPresent(const char *extension) const;

    // Returns true if all the features are present
    template<typename T>
    bool AreExtensionFeaturesPresent(T const &features) const {
        return extended_features_chain_.Match(
                static_cast<VkStructureType>(features.sType), &features);
    }

    // If the given extension is present, make the extension be enabled on the
    // device. Returns true the extension is present.
    bool EnableExtensionIfPresent(const char *extension);

    // If all the given extensions are present, make all the extensions be enabled
    // on the device. Returns true if all the extensions are present.
    bool EnableExtensionsIfPresent(size_t count, const char *const *extensions);

    bool EnableExtensionsIfPresent(const std::vector<const char *> &extensions) {
        return EnableExtensionsIfPresent(extensions.size(), extensions.data());
    }

    // If the features from VkPhysicalDeviceFeatures are all present, make all of
    // the features be enable on the device. Returns true if all the features are
    // present.
    bool EnableFeaturesIfPresent(const VkPhysicalDeviceFeatures &features_to_enable) const;

    // If the features from the provided features struct are all present, make all
    // of the features be enable on the device. Returns true if all of the
    // features are present.
    template<typename T>
    bool EnableExtensionFeaturesIfPresent(T const &features_check) {
        T scratch_space_struct{};
        scratch_space_struct.sType = features_check.sType;
        return EnableFeaturesStructIfPresent(
                static_cast<VkStructureType>(features_check.sType), sizeof(T), &features_check, &scratch_space_struct);
    }

    // A conversion function which allows this PhysicalDevice to be used
    // in places where VkPhysicalDevice would have been used.
    explicit operator VkPhysicalDevice() const;

private:
    uint32_t instance_version_ = VK_API_VERSION_1_0;
    std::vector<std::string> extensions_to_enable_;
    std::vector<std::string> available_extensions_;
    std::vector<VkQueueFamilyProperties> queue_families_;
    FeatureChain extended_features_chain_;

    bool defer_surface_initialization_ = false;
    bool properties2_ext_enabled_ = false;

    enum class Suitable {
        yes, partial, no
    };

    Suitable suitable = Suitable::yes;

    friend class PhysicalDeviceSelector;

    friend class DeviceBuilder;

    bool EnableFeaturesStructIfPresent(VkStructureType sType, size_t struct_size, const void *features_struct,
                                       void *query_struct);
};

enum class PreferredDeviceType {
    kOther = 0,
    kIntegrated = 1,
    kDiscrete = 2,
    kVirtual_gpu = 3,
    kCpu = 4
};

// Enumerates the physical devices on the system, and based on the added
// criteria, returns a physical device or list of physical devies A device is
// considered suitable if it meets all the 'required' criteria.
class PhysicalDeviceSelector {
public:
    // Requires a vkb::Instance to construct, needed to pass instance creation
    // info.
    explicit PhysicalDeviceSelector(Instance const &instance);

    // Requires a vkb::Instance to construct, needed to pass instance creation
    // info, optionally specify the surface here
    explicit PhysicalDeviceSelector(Instance const &instance, VkSurfaceKHR surface);

    // Return the first device which is suitable
    // use the `selection` parameter to configure if partially
    PhysicalDevice Select() const;

    // Return all devices which are considered suitable - intended for
    // applications which want to let the user pick the physical device
    std::vector<PhysicalDevice> SelectDevices() const;

    // Return the names of all devices which are considered suitable - intended
    // for applications which want to let the user pick the physical device
    std::vector<std::string> SelectDeviceNames() const;

    // Set the surface in which the physical device should render to.
    // Be sure to set it if swapchain functionality is to be used.
    PhysicalDeviceSelector &SetSurface(VkSurfaceKHR surface);

    // Set the name of the device to select.
    PhysicalDeviceSelector &SetName(std::string const &name);

    // Set the desired physical device type to select. Defaults to
    // PreferredDeviceType::discrete.
    PhysicalDeviceSelector &PreferGpuDeviceType(
            PreferredDeviceType type = PreferredDeviceType::kDiscrete);

    // Allow selection of a gpu device type that isn't the preferred physical
    // device type. Defaults to true.
    PhysicalDeviceSelector &AllowAnyGpuDeviceType(bool allow_any_type = true);

    // Require that a physical device supports presentation. Defaults to true.
    PhysicalDeviceSelector &RequirePresent(bool require = true);

    // Require a queue family that supports compute operations but not graphics
    // nor transfer.
    PhysicalDeviceSelector &RequireDedicatedComputeQueue();

    // Require a queue family that supports transfer operations but not graphics
    // nor compute.
    PhysicalDeviceSelector &RequireDedicatedTransferQueue();

    // Require a queue family that supports compute operations but not graphics.
    PhysicalDeviceSelector &RequireSeparateComputeQueue();

    // Require a queue family that supports transfer operations but not graphics.
    PhysicalDeviceSelector &RequireSeparateTransferQueue();

    // Require a memory heap from VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT with `size`
    // memory available.
    PhysicalDeviceSelector &RequiredDeviceMemorySize(VkDeviceSize size);

    // Require a physical device which supports a specific extension.
    PhysicalDeviceSelector &AddRequiredExtension(const char *extension);

    // Require a physical device which supports a set of extensions.
    PhysicalDeviceSelector &AddRequiredExtensions(size_t count, const char *const *extensions);

    PhysicalDeviceSelector &add_required_extensions(std::vector<const char *> const &extensions) {
        return AddRequiredExtensions(extensions.size(), extensions.data());
    }


    // Require a physical device that supports a (major, minor) version of vulkan.
    PhysicalDeviceSelector &SetMinimumVersion(uint32_t major, uint32_t minor);

    // By default PhysicalDeviceSelector enables the portability subset if
    // available This function disables that behavior
    PhysicalDeviceSelector &DisablePortabilitySubset();

    // Require a physical device which supports a specific set of
    // general/extension features. If this function is used, the user should not
    // put their own VkPhysicalDeviceFeatures2 in the pNext chain of
    // VkDeviceCreateInfo.
    template<typename T>
    PhysicalDeviceSelector &AddRequiredExtensionFeatures(T const &features) {
        criteria_.extended_features_chain.AddStructure(
                static_cast<VkStructureType>(features.sType), sizeof(T), &features);
        return *this;
    }

    // Require a physical device which supports the features in
    // VkPhysicalDeviceFeatures.
    PhysicalDeviceSelector &SetRequiredFeatures(VkPhysicalDeviceFeatures const &features);

    // Used when surface creation happens after physical device selection.
    // Warning: This disables checking if the physical device supports a given
    // surface.
    PhysicalDeviceSelector &DeferSurfaceInitialization();

    // Ignore all criteria and choose the first physical device that is available.
    // Only use when: The first gpu in the list may be set by global user
    // preferences and an application may wish to respect it.
    PhysicalDeviceSelector &SelectFirstDeviceUnconditionally(bool unconditionally = true);

private:
    struct InstanceInfo {
        VkInstance instance = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        uint32_t version = VK_API_VERSION_1_0;
        bool headless = false;
        bool properties2_ext_enabled = false;
    } instance_info_;

    // We copy the extension features stored in the selector criteria under the
    // prose of a "template" to ensure that after fetching everything is compared
    // 1:1 during a match.

    struct SelectionCriteria {
        std::string name;
        PreferredDeviceType preferred_type = PreferredDeviceType::kDiscrete;
        bool allow_any_type = true;
        bool require_present = true;
        bool require_dedicated_transfer_queue = false;
        bool require_dedicated_compute_queue = false;
        bool require_separate_transfer_queue = false;
        bool require_separate_compute_queue = false;
        VkDeviceSize required_mem_size = 0;

        std::vector<std::string> required_extensions;

        uint32_t required_version = VK_API_VERSION_1_0;

        VkPhysicalDeviceFeatures required_features{};
        VkPhysicalDeviceFeatures2 required_features2{};

        FeatureChain extended_features_chain;
        bool defer_surface_initialization = false;
        bool use_first_gpu_unconditionally = false;
        bool enable_portability_subset = true;
    } criteria_;

    PhysicalDevice
    PopulateDeviceDetails(VkPhysicalDevice phys_device, FeatureChain const &src_extended_features_chain) const;

    PhysicalDevice::Suitable
    IsDeviceSuitable(PhysicalDevice const &phys_device, std::vector<std::string> &unsuitability_reasons) const;
};


// ---- Queue ---- //
enum class QueueType {
    kPresent, kGraphics, kCompute, kTransfer
};
// ---- Device ---- //

struct Device {
    VkDevice device = VK_NULL_HANDLE;
    PhysicalDevice physical_device;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    std::vector<VkQueueFamilyProperties> queue_families;
    VkAllocationCallbacks *allocation_callbacks = nullptr;
    PFN_vkGetDeviceProcAddr fp_vkGetDeviceProcAddr = nullptr;
    uint32_t instance_version = VK_API_VERSION_1_0;

    uint32_t GetQueueIndex(QueueType type) const;

    // Only a compute or transfer queue type is valid. All other queue types do not support a 'dedicated' queue index
    uint32_t GetDedicatedQueueIndex(QueueType type) const;

    VkQueue GetQueue(QueueType type) const;

    // Only a compute or transfer queue type is valid. All other queue types do not support a 'dedicated' queue
    VkQueue GetDedicatedQueue(QueueType type) const;

    // A conversion function which allows this Device to be used
    // in places where VkDevice would have been used.
    explicit operator VkDevice() const;

private:
    struct {
        PFN_vkGetDeviceQueue fp_vkGetDeviceQueue = nullptr;
        PFN_vkDestroyDevice fp_vkDestroyDevice = nullptr;
    } internal_table;

    friend class DeviceBuilder;

    friend void destroy_device(Device const &device);
};


// For advanced device queue setup
struct CustomQueueDescription {
    explicit CustomQueueDescription(uint32_t index, std::vector<float> const &priorities)
            : index(index), priorities(priorities) {}

    explicit CustomQueueDescription(uint32_t index, std::vector<float> &&priorities)
            : index(index), priorities(std::move(priorities)) {}

    explicit CustomQueueDescription(uint32_t index, size_t count, float const *priorities)
            : index(index), priorities(priorities, priorities + count) {}

    uint32_t index;
    std::vector<float> priorities;
};

void destroy_device(Device const &device);

class DeviceBuilder {
public:
    // Any features and extensions that are requested/required in PhysicalDeviceSelector are automatically enabled.
    explicit DeviceBuilder(PhysicalDevice physical_device);

    Device Build() const;

    // For Advanced Users: specify the exact list of VkDeviceQueueCreateInfo's needed for the application.
    // If a custom queue setup is provided, getting the queues and queue indexes is up to the application.
    DeviceBuilder &CustomQueueSetup(size_t count, CustomQueueDescription const *queue_descriptions);

    DeviceBuilder &CustomQueueSetup(std::vector<CustomQueueDescription> const &queue_descriptions);

    DeviceBuilder &CustomQueueSetup(std::vector<CustomQueueDescription> &&queue_descriptions);

#if VKB_SPAN_OVERLOADS
    DeviceBuilder& custom_queue_setup(std::span<const CustomQueueDescription> queue_descriptions);
#endif

    // Add a structure to the pNext chain of VkDeviceCreateInfo.
    // The structure must be valid when DeviceBuilder::build() is called.
    template<typename T>
    DeviceBuilder &AddPNext(T *structure) {
        info.next_chain.push_back(structure);
        return *this;
    }

    // Provide custom allocation callbacks.
    DeviceBuilder &SetAllocationCallbacks(VkAllocationCallbacks *callbacks);

private:
    PhysicalDevice physical_device;
    struct DeviceInfo {
        VkDeviceCreateFlags flags = static_cast<VkDeviceCreateFlags>(0);
        std::vector<void *> next_chain;
        std::vector<CustomQueueDescription> queue_descriptions;
        VkAllocationCallbacks *allocation_callbacks = nullptr;
    } info;
};
} // end namespace lvk

#endif  // LVK_PHYSICAL_DEVICE_H
