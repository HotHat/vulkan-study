//
// Created by admin on 2025/11/26.
//

#ifndef LYH_TEXTURE_H
#define LYH_TEXTURE_H

#include <vulkan/vulkan.h>

#include "allocator.h"
#include "render_context.h"
#include "stb_image.h"
#include "vulkan_context.h"

namespace lvk {
class Texture {
public:
    explicit Texture(RenderContext &context) : context(context) {
        //
    }

    Texture &operator=(Texture &&other) noexcept {
        texture = std::move(other.texture);
        sampler = other.sampler;
        imageView = other.imageView;

        other.imageView = VK_NULL_HANDLE;
        other.sampler = VK_NULL_HANDLE;
        return *this;
    }

    Texture(Texture &&other) noexcept : context(other.context) {
        texture = std::move(other.texture);
        sampler = other.sampler;
        imageView = other.imageView;

        other.imageView = VK_NULL_HANDLE;
        other.sampler = VK_NULL_HANDLE;
    }

    void LoadImage(const std::string &file) {
        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load(file.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        // mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }
        auto texture_buffer = context.GetAllocator().CreateBuffer2(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                   VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                                                                   VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                                                   |
                                                                   VMA_ALLOCATION_CREATE_MAPPED_BIT);
        texture_buffer->CopyData(imageSize, (void *) pixels);
        texture_buffer->Flush(0, imageSize);
        //
        stbi_image_free(pixels);

        texture = context.GetAllocator().CreateImage(
            {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)},
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
        );


        TransitionImageLayout(texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        CopyBufferToImage(texture_buffer->buffer, texture->image, static_cast<uint32_t>(texWidth),
                          static_cast<uint32_t>(texHeight));

        TransitionImageLayout(texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        texture_buffer->Destroy();

        //
        createTextureImageView();
        createTextureSampler();
    }

    VkSampler GetSampler() { return sampler; }
    VkImageView GetImageView() { return imageView; }

    void TransitionImageLayout(VkImage image_, VkFormat format, VkImageLayout oldLayout,
                               VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = context.BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image_;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout ==
                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        context.EndSingleTimeCommands(commandBuffer);
    }

    void CopyBufferToImage(VkBuffer p_buffer, VkImage p_image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = context.BeginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(commandBuffer, p_buffer, p_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        context.EndSingleTimeCommands(commandBuffer);
    }

    void Destroy() const {
        if (texture) {
            texture->Destroy();
        }
        vkDestroySampler(context.GetContext().device.device, sampler, nullptr);
        vkDestroyImageView(context.GetContext().device.device, imageView, nullptr);
    }

private:
    RenderContext &context;

    std::unique_ptr<Image> texture;
    VkSampler sampler{};
    VkImageView imageView = VK_NULL_HANDLE;


    void createTextureImageView() {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = texture->image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(context.GetContext().device.device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image view!");
        }
    }

    void createTextureSampler() {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(context.GetContext().device.physical_device.physical_device, &properties);

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

        if (vkCreateSampler(context.GetContext().device.device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }
};
} // end namespace lvk
#endif //LYH_TEXTURE_H
