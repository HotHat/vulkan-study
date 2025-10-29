//
// Created by admin on 2025/10/29.
//

#include "swapchain.h"
#include <stdexcept>

namespace lvk {
struct SurfaceSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

enum class SurfaceSupportError {
    surface_handle_null,
    failed_get_surface_capabilities,
    failed_enumerate_surface_formats,
    failed_enumerate_present_modes,
    no_suitable_desired_format
};



SurfaceSupportDetails query_surface_support_details(VkPhysicalDevice phys_device, VkSurfaceKHR surface) {
    if (surface == VK_NULL_HANDLE) {
        throw std::runtime_error("surface_handle_null");
    }

    VkSurfaceCapabilitiesKHR capabilities;
    VkResult res = detail::vulkan_functions().fp_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_device, surface, &capabilities);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("failed_get_surface_capabilities");
    }

    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;

    auto formats_ret = get_vector<VkSurfaceFormatKHR>(
            formats, vkGetPhysicalDeviceSurfaceFormatsKHR, phys_device, surface);
    if (formats_ret != VK_SUCCESS)
        return { make_error_code(SurfaceSupportError::failed_enumerate_surface_formats), formats_ret };
    auto present_modes_ret = detail::get_vector<VkPresentModeKHR>(
            present_modes, detail::vulkan_functions().fp_vkGetPhysicalDeviceSurfacePresentModesKHR, phys_device, surface);
    if (present_modes_ret != VK_SUCCESS)
        return { make_error_code(SurfaceSupportError::failed_enumerate_present_modes), present_modes_ret };

    return SurfaceSupportDetails{ capabilities, formats, present_modes };
}

VkSurfaceFormatKHR find_desired_surface_format(
        std::vector<VkSurfaceFormatKHR> const& available_formats, std::vector<VkSurfaceFormatKHR> const& desired_formats) {
    for (auto const& desired_format : desired_formats) {
        for (auto const& available_format : available_formats) {
            // finds the first format that is desired and available
            if (desired_format.format == available_format.format && desired_format.colorSpace == available_format.colorSpace) {
                return desired_format;
            }
        }
    }

    // if no desired format is available, we report that no format is suitable to the user request
    return { make_error_code(SurfaceSupportError::no_suitable_desired_format) };
}

VkSurfaceFormatKHR find_best_surface_format(
        std::vector<VkSurfaceFormatKHR> const& available_formats, std::vector<VkSurfaceFormatKHR> const& desired_formats) {
    auto surface_format_ret = detail::find_desired_surface_format(available_formats, desired_formats);
    if (surface_format_ret.has_value()) return surface_format_ret.value();

    // use the first available format as a fallback if any desired formats aren't found
    return available_formats[0];
}

VkPresentModeKHR find_present_mode(std::vector<VkPresentModeKHR> const& available_resent_modes,
                                   std::vector<VkPresentModeKHR> const& desired_present_modes) {
    for (auto const& desired_pm : desired_present_modes) {
        for (auto const& available_pm : available_resent_modes) {
            // finds the first present mode that is desired and available
            if (desired_pm == available_pm) return desired_pm;
        }
    }
    // only present mode required, use as a fallback
    return VK_PRESENT_MODE_FIFO_KHR;
}

template <typename T> T minimum(T a, T b) { return a < b ? a : b; }
template <typename T> T maximum(T a, T b) { return a > b ? a : b; }

VkExtent2D find_extent(VkSurfaceCapabilitiesKHR const& capabilities, uint32_t desired_width, uint32_t desired_height) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = { desired_width, desired_height };

        actualExtent.width =
                maximum(capabilities.minImageExtent.width, minimum(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height =
                maximum(capabilities.minImageExtent.height, minimum(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}
} // namespace detail

void destroy_swapchain(Swapchain const& swapchain) {
    if (swapchain.device != VK_NULL_HANDLE && swapchain.swapchain != VK_NULL_HANDLE) {
        swapchain.internal_table.fp_vkDestroySwapchainKHR(swapchain.device, swapchain.swapchain, swapchain.allocation_callbacks);
    }
}

SwapchainBuilder::SwapchainBuilder(Device const& device) {
    info.physical_device = device.physical_device.physical_device;
    info.device = device.device;
    info.surface = device.surface;
    info.instance_version = device.instance_version;
    auto present = device.get_queue_index(QueueType::present);
    auto graphics = device.get_queue_index(QueueType::graphics);
    assert(graphics.has_value() && present.has_value() && "Graphics and Present queue indexes must be valid");
    info.graphics_queue_index = present.value();
    info.present_queue_index = graphics.value();
}
SwapchainBuilder::SwapchainBuilder(Device const& device, VkSurfaceKHR const surface) {
    info.physical_device = device.physical_device.physical_device;
    info.device = device.device;
    info.surface = surface;
    info.instance_version = device.instance_version;
    Device temp_device = device;
    temp_device.surface = surface;
    auto present = temp_device.get_queue_index(QueueType::present);
    auto graphics = temp_device.get_queue_index(QueueType::graphics);
    assert(graphics.has_value() && present.has_value() && "Graphics and Present queue indexes must be valid");
    info.graphics_queue_index = graphics.value();
    info.present_queue_index = present.value();
}
SwapchainBuilder::SwapchainBuilder(VkPhysicalDevice const physical_device,
                                   VkDevice const device,
                                   VkSurfaceKHR const surface,
                                   uint32_t graphics_queue_index,
                                   uint32_t present_queue_index) {
    info.physical_device = physical_device;
    info.device = device;
    info.surface = surface;
    info.graphics_queue_index = graphics_queue_index;
    info.present_queue_index = present_queue_index;
    if (graphics_queue_index == detail::QUEUE_INDEX_MAX_VALUE || present_queue_index == detail::QUEUE_INDEX_MAX_VALUE) {
        auto queue_families = detail::get_vector_noerror<VkQueueFamilyProperties>(
                detail::vulkan_functions().fp_vkGetPhysicalDeviceQueueFamilyProperties, physical_device);
        if (graphics_queue_index == detail::QUEUE_INDEX_MAX_VALUE)
            info.graphics_queue_index = detail::get_first_queue_index(queue_families, VK_QUEUE_GRAPHICS_BIT);
        if (present_queue_index == detail::QUEUE_INDEX_MAX_VALUE)
            info.present_queue_index = detail::get_present_queue_index(physical_device, surface, queue_families);
    }
}
Result<Swapchain> SwapchainBuilder::build() const {
    if (info.surface == VK_NULL_HANDLE) {
        return Result<Swapchain>{ SwapchainError::surface_handle_not_provided };
    }

    auto desired_formats = info.desired_formats;
    if (desired_formats.empty()) add_desired_formats(desired_formats);
    auto desired_present_modes = info.desired_present_modes;
    if (desired_present_modes.empty()) add_desired_present_modes(desired_present_modes);

    auto surface_support_ret = detail::query_surface_support_details(info.physical_device, info.surface);
    if (!surface_support_ret.has_value())
        return Result<Swapchain>{ SwapchainError::failed_query_surface_support_details, surface_support_ret.vk_result() };
    auto surface_support = surface_support_ret.value();

    uint32_t image_count = info.min_image_count;
    if (info.required_min_image_count >= 1) {
        if (info.required_min_image_count < surface_support.capabilities.minImageCount)
            return make_error_code(SwapchainError::required_min_image_count_too_low);

        image_count = info.required_min_image_count;
    } else if (info.min_image_count == 0) {
        // We intentionally use minImageCount + 1 to maintain existing behavior, even if it typically results in triple buffering on most systems.
        image_count = surface_support.capabilities.minImageCount + 1;
    } else {
        image_count = info.min_image_count;
        if (image_count < surface_support.capabilities.minImageCount)
            image_count = surface_support.capabilities.minImageCount;
    }
    if (surface_support.capabilities.maxImageCount > 0 && image_count > surface_support.capabilities.maxImageCount) {
        image_count = surface_support.capabilities.maxImageCount;
    }

    VkSurfaceFormatKHR surface_format = detail::find_best_surface_format(surface_support.formats, desired_formats);

    VkExtent2D extent = detail::find_extent(surface_support.capabilities, info.desired_width, info.desired_height);

    uint32_t image_array_layers = info.array_layer_count;
    if (surface_support.capabilities.maxImageArrayLayers < info.array_layer_count)
        image_array_layers = surface_support.capabilities.maxImageArrayLayers;
    if (info.array_layer_count == 0) image_array_layers = 1;

    uint32_t queue_family_indices[] = { info.graphics_queue_index, info.present_queue_index };


    VkPresentModeKHR present_mode = detail::find_present_mode(surface_support.present_modes, desired_present_modes);

    // VkSurfaceCapabilitiesKHR::supportedUsageFlags is only only valid for some present modes. For shared present modes, we should also check VkSharedPresentSurfaceCapabilitiesKHR::sharedPresentSupportedUsageFlags.
    auto is_unextended_present_mode = [](VkPresentModeKHR present_mode) {
        return (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR) || (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) ||
               (present_mode == VK_PRESENT_MODE_FIFO_KHR) || (present_mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR);
    };

    if (is_unextended_present_mode(present_mode) &&
        (info.image_usage_flags & surface_support.capabilities.supportedUsageFlags) != info.image_usage_flags) {
        return Result<Swapchain>{ SwapchainError::required_usage_not_supported };
    }

    VkSurfaceTransformFlagBitsKHR pre_transform = info.pre_transform;
    if (info.pre_transform == static_cast<VkSurfaceTransformFlagBitsKHR>(0))
        pre_transform = surface_support.capabilities.currentTransform;

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    detail::setup_pNext_chain(swapchain_create_info, info.pNext_chain);

    swapchain_create_info.flags = info.create_flags;
    swapchain_create_info.surface = info.surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = surface_format.format;
    swapchain_create_info.imageColorSpace = surface_format.colorSpace;
    swapchain_create_info.imageExtent = extent;
    swapchain_create_info.imageArrayLayers = image_array_layers;
    swapchain_create_info.imageUsage = info.image_usage_flags;

    if (info.graphics_queue_index != info.present_queue_index) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    swapchain_create_info.preTransform = pre_transform;
    swapchain_create_info.compositeAlpha = info.composite_alpha;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = info.clipped;
    swapchain_create_info.oldSwapchain = info.old_swapchain;
    Swapchain swapchain{};
    PFN_vkCreateSwapchainKHR swapchain_create_proc;
    detail::vulkan_functions().get_device_proc_addr(info.device, swapchain_create_proc, "vkCreateSwapchainKHR");
    auto res = swapchain_create_proc(info.device, &swapchain_create_info, info.allocation_callbacks, &swapchain.swapchain);

    if (res != VK_SUCCESS) {
        return Result<Swapchain>{ SwapchainError::failed_create_swapchain, res };
    }
    swapchain.device = info.device;
    swapchain.image_format = surface_format.format;
    swapchain.color_space = surface_format.colorSpace;
    swapchain.image_usage_flags = info.image_usage_flags;
    swapchain.extent = extent;
    detail::vulkan_functions().get_device_proc_addr(
            info.device, swapchain.internal_table.fp_vkGetSwapchainImagesKHR, "vkGetSwapchainImagesKHR");
    detail::vulkan_functions().get_device_proc_addr(info.device, swapchain.internal_table.fp_vkCreateImageView, "vkCreateImageView");
    detail::vulkan_functions().get_device_proc_addr(info.device, swapchain.internal_table.fp_vkDestroyImageView, "vkDestroyImageView");
    detail::vulkan_functions().get_device_proc_addr(
            info.device, swapchain.internal_table.fp_vkDestroySwapchainKHR, "vkDestroySwapchainKHR");
    auto images = swapchain.get_images();
    if (!images) {
        return Result<Swapchain>{ SwapchainError::failed_get_swapchain_images };
    }
    swapchain.requested_min_image_count = image_count;
    swapchain.present_mode = present_mode;
    swapchain.image_count = static_cast<uint32_t>(images.value().size());
    swapchain.instance_version = info.instance_version;
    swapchain.allocation_callbacks = info.allocation_callbacks;
    return swapchain;
}
Result<std::vector<VkImage>> Swapchain::get_images() {
    std::vector<VkImage> swapchain_images;

    auto swapchain_images_ret =
            detail::get_vector<VkImage>(swapchain_images, internal_table.fp_vkGetSwapchainImagesKHR, device, swapchain);
    if (swapchain_images_ret != VK_SUCCESS) {
        return Result<std::vector<VkImage>>{ SwapchainError::failed_get_swapchain_images, swapchain_images_ret };
    }
    return swapchain_images;
}
Result<std::vector<VkImageView>> Swapchain::get_image_views() { return get_image_views(nullptr); }
Result<std::vector<VkImageView>> Swapchain::get_image_views(const void* pNext) {
    const auto swapchain_images_ret = get_images();
    if (!swapchain_images_ret) return swapchain_images_ret.error();
    const auto& swapchain_images = swapchain_images_ret.value();

    bool already_contains_image_view_usage = false;
    while (pNext) {
        if (reinterpret_cast<const VkBaseInStructure*>(pNext)->sType == VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO) {
            already_contains_image_view_usage = true;
            break;
        }
        pNext = reinterpret_cast<const VkBaseInStructure*>(pNext)->pNext;
    }
    VkImageViewUsageCreateInfo desired_flags{};
    desired_flags.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
    desired_flags.pNext = pNext;
    desired_flags.usage = image_usage_flags;

    std::vector<VkImageView> views(swapchain_images.size());
    for (size_t i = 0; i < swapchain_images.size(); i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        if (instance_version >= VKB_VK_API_VERSION_1_1 && !already_contains_image_view_usage) {
            createInfo.pNext = &desired_flags;
        } else {
            createInfo.pNext = pNext;
        }

        createInfo.image = swapchain_images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = image_format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        VkResult res = internal_table.fp_vkCreateImageView(device, &createInfo, allocation_callbacks, &views[i]);
        if (res != VK_SUCCESS) {
            // Cleanup already created image views
            destroy_image_views(i, views.data());
            return Result<std::vector<VkImageView>>{ SwapchainError::failed_create_swapchain_image_views, res };
        }
    }
    return views;
}
void Swapchain::destroy_image_views(size_t count, VkImageView const* image_views) {
    for (size_t i = 0; i < count; ++i) {
        internal_table.fp_vkDestroyImageView(device, image_views[i], allocation_callbacks);
    }
}
void Swapchain::destroy_image_views(std::vector<VkImageView> const& image_views) {
    destroy_image_views(image_views.size(), image_views.data());
}
#if VKB_SPAN_OVERLOADS
void Swapchain::destroy_image_views(std::span<const VkImageView> image_views) {
    destroy_image_views(image_views.size(), image_views.data());
}
#endif
Swapchain::operator VkSwapchainKHR() const { return this->swapchain; }
SwapchainBuilder& SwapchainBuilder::set_old_swapchain(VkSwapchainKHR old_swapchain) {
    info.old_swapchain = old_swapchain;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::set_old_swapchain(Swapchain const& swapchain) {
    info.old_swapchain = swapchain.swapchain;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::set_desired_extent(uint32_t width, uint32_t height) {
    info.desired_width = width;
    info.desired_height = height;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::set_desired_format(VkSurfaceFormatKHR format) {
    info.desired_formats.insert(info.desired_formats.begin(), format);
    return *this;
}
SwapchainBuilder& SwapchainBuilder::add_fallback_format(VkSurfaceFormatKHR format) {
    info.desired_formats.push_back(format);
    return *this;
}
SwapchainBuilder& SwapchainBuilder::use_default_format_selection() {
    info.desired_formats.clear();
    add_desired_formats(info.desired_formats);
    return *this;
}

SwapchainBuilder& SwapchainBuilder::set_desired_present_mode(VkPresentModeKHR present_mode) {
    info.desired_present_modes.insert(info.desired_present_modes.begin(), present_mode);
    return *this;
}
SwapchainBuilder& SwapchainBuilder::add_fallback_present_mode(VkPresentModeKHR present_mode) {
    info.desired_present_modes.push_back(present_mode);
    return *this;
}
SwapchainBuilder& SwapchainBuilder::use_default_present_mode_selection() {
    info.desired_present_modes.clear();
    add_desired_present_modes(info.desired_present_modes);
    return *this;
}
SwapchainBuilder& SwapchainBuilder::set_allocation_callbacks(VkAllocationCallbacks* callbacks) {
    info.allocation_callbacks = callbacks;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::set_image_usage_flags(VkImageUsageFlags usage_flags) {
    info.image_usage_flags = usage_flags;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::add_image_usage_flags(VkImageUsageFlags usage_flags) {
    info.image_usage_flags = info.image_usage_flags | usage_flags;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::use_default_image_usage_flags() {
    info.image_usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::set_image_array_layer_count(uint32_t array_layer_count) {
    info.array_layer_count = array_layer_count;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::set_desired_min_image_count(uint32_t min_image_count) {
    info.min_image_count = min_image_count;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::set_required_min_image_count(uint32_t required_min_image_count) {
    info.required_min_image_count = required_min_image_count;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::set_clipped(bool clipped) {
    info.clipped = clipped;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::set_create_flags(VkSwapchainCreateFlagBitsKHR create_flags) {
    info.create_flags = create_flags;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::set_pre_transform_flags(VkSurfaceTransformFlagBitsKHR pre_transform_flags) {
    info.pre_transform = pre_transform_flags;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::set_composite_alpha_flags(VkCompositeAlphaFlagBitsKHR composite_alpha_flags) {
    info.composite_alpha = composite_alpha_flags;
    return *this;
}

void SwapchainBuilder::add_desired_formats(std::vector<VkSurfaceFormatKHR>& formats) const {
    formats.push_back({ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR });
    formats.push_back({ VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR });
}
void SwapchainBuilder::add_desired_present_modes(std::vector<VkPresentModeKHR>& modes) const {
    modes.push_back(VK_PRESENT_MODE_MAILBOX_KHR);
    modes.push_back(VK_PRESENT_MODE_FIFO_KHR);
}
} // end namespace lvk