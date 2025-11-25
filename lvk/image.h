//
// Created by admin on 2025/11/13.
//

#ifndef LYH_IMAGE_H
#define LYH_IMAGE_H
#include <stdexcept>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "vulkan_context.h"

namespace lvk {
struct Image {
    explicit Image(VulkanContext &context, VmaAllocator &allocator_, VkImage image_, VmaAllocation allocation_): context(context), allocator(allocator_),
        image(image_),
        allocation(allocation_) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(context.device.device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image view!");
        }

        //
        createTextureSampler();
    }

    void Destroy() const {
        vkDestroyImageView(context.device.device, imageView, nullptr);
        vkDestroySampler(context.device.device, sampler, nullptr);
        vmaDestroyImage(allocator, image, allocation);
    }

    void CopyData(uint32_t p_size, void *data) {
        void *mappedData;
        vmaMapMemory(allocator, allocation, &mappedData);
        memcpy(mappedData, data, p_size);
        vmaUnmapMemory(allocator, allocation);

        size = p_size;
    }

    void Flush(VkDeviceSize offset, VkDeviceSize size) const {
        vmaFlushAllocation(allocator, allocation, offset, size);
    }

    void Flush() const {
        vmaFlushAllocation(allocator, allocation, 0, VK_WHOLE_SIZE);
    }

    // ~Buffer() {
    //     VmaAllocation allocation = nullptr;
    //     vmaDestroyBuffer(allocator, buffer, allocation);
    // }
    size_t size{};
    VulkanContext &context;
    VmaAllocator &allocator;
    VkImage image;
    VkSampler sampler{};
    VkImageView imageView = VK_NULL_HANDLE;
    VmaAllocation allocation;

private:
    void createTextureSampler() {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(context.device.physical_device.physical_device, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        if (vkCreateSampler(context.device.device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }
};
} // end namespace lvk
#endif //LYH_IMAGE_H
