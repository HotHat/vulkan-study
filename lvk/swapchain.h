//
// Created by admin on 2025/10/29.
//

#ifndef LVK_SWAP_CHAIN_H
#define LVK_SWAP_CHAIN_H

#include <vulkan/vulkan.h>
#include <vector>
#include "device.h"

namespace lvk {

struct Swapchain {
    VkDevice device = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    uint32_t image_count = 0;
    VkFormat image_format = VK_FORMAT_UNDEFINED; // The image format actually used when creating the swapchain.
    VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; // The color space actually used when creating the swapchain.
    VkImageUsageFlags image_usage_flags = 0;
    VkExtent2D extent = { 0, 0 };
    // The value of minImageCount actually used when creating the swapchain; note that the presentation engine is always free to create more images than that.
    uint32_t requested_min_image_count = 0;
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR; // The present mode actually used when creating the swapchain.
    uint32_t instance_version = VK_API_VERSION_1_0;
    VkAllocationCallbacks* allocation_callbacks = nullptr;

    // Returns a vector of VkImage handles to the swapchain.
    std::vector<VkImage> GetImages();

    // Returns a vector of VkImageView's to the VkImage's of the swapchain.
    // VkImageViews must be destroyed.  The pNext chain must be a nullptr or a valid
    // structure.
    std::vector<VkImageView> GetImageViews();
    std::vector<VkImageView> GetImageViews(const void* pNext);
    void DestroyImageViews(size_t count, VkImageView const* image_views) const;
    void DestroyImageViews(std::vector<VkImageView> const& image_views) const;
    void DestroyImageViews() const;

    // A conversion function which allows this Swapchain to be used
    // in places where VkSwapchainKHR would have been used.
    explicit operator VkSwapchainKHR() const;

private:
    struct {
        PFN_vkGetSwapchainImagesKHR fp_vkGetSwapchainImagesKHR = nullptr;
        PFN_vkCreateImageView fp_vkCreateImageView = nullptr;
        PFN_vkDestroyImageView fp_vkDestroyImageView = nullptr;
        PFN_vkDestroySwapchainKHR fp_vkDestroySwapchainKHR = nullptr;
    } internal_table;
    friend class SwapchainBuilder;
    friend void destroy_swapchain(Swapchain const& swapchain);
};

void destroy_swapchain(Swapchain const& swapchain);

class SwapchainBuilder {
public:
    // Construct a SwapchainBuilder with a `vkb::Device`
    explicit SwapchainBuilder(Device const& device);
    // Construct a SwapchainBuilder with a specific VkSurfaceKHR handle and `vkb::Device`
    explicit SwapchainBuilder(Device const& device, VkSurfaceKHR const surface);
    // Construct a SwapchainBuilder with Vulkan handles for the physical device, device, and surface
    // Optionally can provide the uint32_t indices for the graphics and present queue
    // Note: The constructor will query the graphics & present queue if the indices are not provided
    explicit SwapchainBuilder(VkPhysicalDevice const physical_device,
                              VkDevice const device,
                              VkSurfaceKHR const surface,
                              uint32_t graphics_queue_index = QUEUE_INDEX_MAX_VALUE,
                              uint32_t present_queue_index = QUEUE_INDEX_MAX_VALUE);

    Swapchain Build() const;

    // Set the oldSwapchain member of VkSwapchainCreateInfoKHR.
    // For use in rebuilding a swapchain.
    SwapchainBuilder& SetOldSwapchain(VkSwapchainKHR old_swapchain);
    SwapchainBuilder& SetOldSwapchain(Swapchain const& swapchain);


    // Desired size of the swapchain. By default, the swapchain will use the size
    // of the window being drawn to.
    SwapchainBuilder& SetDesiredExtent(uint32_t width, uint32_t height);

    // When determining the surface format, make this the first to be used if supported.
    SwapchainBuilder& SetDesiredFormat(VkSurfaceFormatKHR format);
    // Add this swapchain format to the end of the list of formats selected from.
    SwapchainBuilder& AddFallbackFormat(VkSurfaceFormatKHR format);
    // Use the default swapchain formats. This is done if no formats are provided.
    // Default surface format is {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
    SwapchainBuilder& UseDefaultFormatSelection();

    // When determining the present mode, make this the first to be used if supported.
    SwapchainBuilder& SetDesiredPresentMode(VkPresentModeKHR present_mode);
    // Add this present mode to the end of the list of present modes selected from.
    SwapchainBuilder& AddFallbackPresentMode(VkPresentModeKHR present_mode);
    // Use the default presentation mode. This is done if no present modes are provided.
    // Default present modes: VK_PRESENT_MODE_MAILBOX_KHR with fallback VK_PRESENT_MODE_FIFO_KHR
    SwapchainBuilder& UseDefaultPresentModeSelection();

    // Set the bitmask of the image usage for acquired swapchain images.
    // If the surface capabilities cannot allow it, building the swapchain will result in the `SwapchainError::required_usage_not_supported` error.
    SwapchainBuilder& SetImageUsageFlags(VkImageUsageFlags usage_flags);
    // Add a image usage to the bitmask for acquired swapchain images.
    SwapchainBuilder& AddImageUsageFlags(VkImageUsageFlags usage_flags);
    // Use the default image usage bitmask values. This is the default if no image usages
    // are provided. The default is VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    SwapchainBuilder& UseDefaultImageUsageFlags();

    // Set the number of views in for multiview/stereo surface
    SwapchainBuilder& SetImageArrayLayerCount(uint32_t array_layer_count);

    // Convenient named constants for passing to set_desired_min_image_count().
    // Note that it is not an `enum class`, so its constants can be passed as an integer value without casting
    // In other words, these might as well be `static const int`, but they benefit from being grouped together this way.
    enum BufferMode {
        kSingleBuffering = 1,
        kDoubleBuffering = 2,
        kTripleBuffering = 3,
    };

    // Sets the desired minimum image count for the swapchain.
    // Note that the presentation engine is always free to create more images than requested.
    // You may pass one of the values specified in the BufferMode enum, or any integer value.
    // For instance, if you pass DOUBLE_BUFFERING, the presentation engine is allowed to give you a double buffering setup, triple buffering, or more. This is up to the drivers.
    SwapchainBuilder& SetDesiredMinImageCount(uint32_t min_image_count);

    // Sets a required minimum image count for the swapchain.
    // If the surface capabilities cannot allow it, building the swapchain will result in the `SwapchainError::required_min_image_count_too_low` error.
    // Otherwise, the same observations from set_desired_min_image_count() apply.
    // A value of 0 is specially interpreted as meaning "no requirement", and is the behavior by default.
    SwapchainBuilder& SetRequiredMinImageCount(uint32_t required_min_image_count);

    // Set whether the Vulkan implementation is allowed to discard rendering operations that
    // affect regions of the surface that are not visible. Default is true.
    // Note: Applications should use the default of true if they do not expect to read back the content
    // of presentable images before presenting them or after reacquiring them, and if their fragment
    // shaders do not have any side effects that require them to run for all pixels in the presentable image.
    SwapchainBuilder& SetClipped(bool clipped = true);

    // Set the VkSwapchainCreateFlagBitsKHR.
    SwapchainBuilder& SetCreateFlags(VkSwapchainCreateFlagBitsKHR create_flags);
    // Set the transform to be applied, like a 90 degree rotation. Default is no transform.
    SwapchainBuilder& SetPreTransformFlags(VkSurfaceTransformFlagBitsKHR pre_transform_flags);
    // Set the alpha channel to be used with other windows in on the system. Default is VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
    SwapchainBuilder& SetCompositeAlphaFlags(VkCompositeAlphaFlagBitsKHR composite_alpha_flags);

    // Add a structure to the pNext chain of VkSwapchainCreateInfoKHR.
    // The structure must be valid when SwapchainBuilder::build() is called.
    template <typename T> SwapchainBuilder& AddNext(T* structure) {
        info.pNext_chain.push_back(structure);
        return *this;
    }

    // Provide custom allocation callbacks.
    SwapchainBuilder& SetAllocationCallbacks(VkAllocationCallbacks* callbacks);

private:
    void AddDesiredFormats(std::vector<VkSurfaceFormatKHR>& formats) const;
    void AddDesiredPresentModes(std::vector<VkPresentModeKHR>& modes) const;

    struct SwapchainInfo {
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        std::vector<void*> pNext_chain;
        VkSwapchainCreateFlagBitsKHR create_flags = static_cast<VkSwapchainCreateFlagBitsKHR>(0);
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        std::vector<VkSurfaceFormatKHR> desired_formats;
        uint32_t instance_version = VK_API_VERSION_1_0;
        uint32_t desired_width = 256;
        uint32_t desired_height = 256;
        uint32_t array_layer_count = 1;
        uint32_t min_image_count = 0;
        uint32_t required_min_image_count = 0;
        VkImageUsageFlags image_usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        uint32_t graphics_queue_index = 0;
        uint32_t present_queue_index = 0;
        VkSurfaceTransformFlagBitsKHR pre_transform = static_cast<VkSurfaceTransformFlagBitsKHR>(0);
#if defined(__ANDROID__)
        VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
#else
        VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
#endif
        std::vector<VkPresentModeKHR> desired_present_modes;
        bool clipped = true;
        VkSwapchainKHR old_swapchain = VK_NULL_HANDLE;
        VkAllocationCallbacks* allocation_callbacks = nullptr;
    } info;
};

} // end namespace lvk

#endif //LVK_SWAP_CHAIN_H
