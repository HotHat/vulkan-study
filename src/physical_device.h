//
// Created by lyhux on 2025/10/27.
//

#ifndef LVK_PHYSICAL_DEVICE_H
#define LVK_PHYSICAL_DEVICE_H
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
        std::vector<VkQueueFamilyProperties> get_queue_families() const;

        // Query the list of extensions which should be enabled
        std::vector<std::string> get_extensions() const;

        // Query the list of extensions which the physical device supports
        std::vector<std::string> get_available_extensions() const;

        // Returns true if an extension should be enabled on the device
        bool IsExtensionPresent(const char *extension) const;

        // Returns true if all the features are present
        template<typename T>
        bool AreExtensionFeaturesPresent(T const &features) const {
            return extended_features_chain_.match(
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
        bool EnableFeaturesIfPresent(
            const VkPhysicalDeviceFeatures &features_to_enable);

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
        operator VkPhysicalDevice() const;

    private:
        uint32_t instance_version_ = VK_API_VERSION_1_0;
        std::vector<std::string> extensions_to_enable_;
        std::vector<std::string> available_extensions_;
        std::vector<VkQueueFamilyProperties> queue_families_;
        detail::FeaturesChain extended_features_chain_;

        bool defer_surface_initialization_ = false;
        bool properties2_ext_enabled_ = false;

        enum class Suitable { yes, partial, no };

        Suitable suitable = Suitable::yes;
        friend class PhysicalDeviceSelector;
        friend class DeviceBuilder;

        bool enable_features_struct_if_present(VkStructureType sType,
                                               size_t struct_size,
                                               const void *features_struct,
                                               void *query_struct);
    };

    enum class PreferredDeviceType {
        other = 0,
        integrated = 1,
        discrete = 2,
        virtual_gpu = 3,
        cpu = 4
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
        explicit PhysicalDeviceSelector(Instance const &instance,
                                        VkSurfaceKHR surface);

        // Return the first device which is suitable
        // use the `selection` parameter to configure if partially
        Result<PhysicalDevice> select() const;

        // Return all devices which are considered suitable - intended for
        // applications which want to let the user pick the physical device
        Result<std::vector<PhysicalDevice> > select_devices() const;

        // Return the names of all devices which are considered suitable - intended
        // for applications which want to let the user pick the physical device
        Result<std::vector<std::string> > select_device_names() const;

        // Set the surface in which the physical device should render to.
        // Be sure to set it if swapchain functionality is to be used.
        PhysicalDeviceSelector &set_surface(VkSurfaceKHR surface);

        // Set the name of the device to select.
        PhysicalDeviceSelector &set_name(std::string const &name);

        // Set the desired physical device type to select. Defaults to
        // PreferredDeviceType::discrete.
        PhysicalDeviceSelector &prefer_gpu_device_type(
            PreferredDeviceType type = PreferredDeviceType::discrete);

        // Allow selection of a gpu device type that isn't the preferred physical
        // device type. Defaults to true.
        PhysicalDeviceSelector &allow_any_gpu_device_type(bool allow_any_type = true);

        // Require that a physical device supports presentation. Defaults to true.
        PhysicalDeviceSelector &require_present(bool require = true);

        // Require a queue family that supports compute operations but not graphics
        // nor transfer.
        PhysicalDeviceSelector &require_dedicated_compute_queue();

        // Require a queue family that supports transfer operations but not graphics
        // nor compute.
        PhysicalDeviceSelector &require_dedicated_transfer_queue();

        // Require a queue family that supports compute operations but not graphics.
        PhysicalDeviceSelector &require_separate_compute_queue();

        // Require a queue family that supports transfer operations but not graphics.
        PhysicalDeviceSelector &require_separate_transfer_queue();

        // Require a memory heap from VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT with `size`
        // memory available.
        PhysicalDeviceSelector &required_device_memory_size(VkDeviceSize size);

        // Require a physical device which supports a specific extension.
        PhysicalDeviceSelector &add_required_extension(const char *extension);

        // Require a physical device which supports a set of extensions.
        PhysicalDeviceSelector &add_required_extensions(
            size_t count, const char *const *extensions);

        PhysicalDeviceSelector &add_required_extensions(
            std::vector<const char *> const &extensions) {
            return add_required_extensions(extensions.size(), extensions.data());
        }


        // Require a physical device that supports a (major, minor) version of vulkan.
        PhysicalDeviceSelector &set_minimum_version(uint32_t major, uint32_t minor);

        // By default PhysicalDeviceSelector enables the portability subset if
        // available This function disables that behavior
        PhysicalDeviceSelector &disable_portability_subset();

        // Require a physical device which supports a specific set of
        // general/extension features. If this function is used, the user should not
        // put their own VkPhysicalDeviceFeatures2 in the pNext chain of
        // VkDeviceCreateInfo.
        template<typename T>
        PhysicalDeviceSelector &add_required_extension_features(T const &features) {
            criteria.extended_features_chain.add_structure(
                static_cast<VkStructureType>(features.sType), sizeof(T), &features);
            return *this;
        }

        // Require a physical device which supports the features in
        // VkPhysicalDeviceFeatures.
        PhysicalDeviceSelector &set_required_features(
            VkPhysicalDeviceFeatures const &features);

        // Used when surface creation happens after physical device selection.
        // Warning: This disables checking if the physical device supports a given
        // surface.
        PhysicalDeviceSelector &defer_surface_initialization();

        // Ignore all criteria and choose the first physical device that is available.
        // Only use when: The first gpu in the list may be set by global user
        // preferences and an application may wish to respect it.
        PhysicalDeviceSelector &select_first_device_unconditionally(
            bool unconditionally = true);

    private:
        struct InstanceInfo {
            VkInstance instance = VK_NULL_HANDLE;
            VkSurfaceKHR surface = VK_NULL_HANDLE;
            uint32_t version = VK_API_VERSION_1_0;
            bool headless = false;
            bool properties2_ext_enabled = false;
        } instance_info;

        // We copy the extension features stored in the selector criteria under the
        // prose of a "template" to ensure that after fetching everything is compared
        // 1:1 during a match.

        struct SelectionCriteria {
            std::string name;
            PreferredDeviceType preferred_type = PreferredDeviceType::discrete;
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

            detail::FeaturesChain extended_features_chain;
            bool defer_surface_initialization = false;
            bool use_first_gpu_unconditionally = false;
            bool enable_portability_subset = true;
        } criteria;

        PhysicalDevice populate_device_details(
            VkPhysicalDevice phys_device,
            detail::FeaturesChain const &src_extended_features_chain) const;

        PhysicalDevice::Suitable is_device_suitable(
            PhysicalDevice const &phys_device,
            std::vector<std::string> &unsuitability_reasons) const;
    };
} // end namespace lvk

#endif  // LVK_PHYSICAL_DEVICE_H
