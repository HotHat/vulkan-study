//
// Created by admin on 2025/11/13.
//

#define VMA_IMPLEMENTATION
#include "allocator.h"

namespace lvk {
Allocator::Allocator(VulkanContext &context_) {
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_1;
    allocatorCreateInfo.physicalDevice = context_.device.physical_device.physical_device;
    allocatorCreateInfo.device = context_.device.device;
    allocatorCreateInfo.instance = context_.instance.instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    vmaCreateAllocator(&allocatorCreateInfo, &allocator);
}

Allocator::~Allocator() {
    // vmaDestroyAllocator(allocator);
}

std::unique_ptr<Buffer> Allocator::CreateBuffer(VkDeviceSize size_, uint32_t usage_, VmaMemoryUsage memory_) {
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = size_;
    bufferInfo.usage = usage_;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memory_;
    // allocInfo.memoryTypeBits = memory_;

    VkBuffer buffer;
    VmaAllocation allocation;
    vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);

    return std::make_unique<Buffer>(allocator, buffer, allocation);
}

std::unique_ptr<Buffer> Allocator::CreateBuffer2(VkDeviceSize p_buffer_size, uint32_t p_buffer_usage,
                                                 VmaMemoryUsage p_alloc_usage, uint32_t p_alloc_flag) {
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = p_buffer_size;
    bufferInfo.usage = p_buffer_usage;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = p_alloc_usage;
    allocInfo.flags = p_alloc_flag;
    // allocInfo.memoryTypeBits = memory_;

    VkBuffer buffer;
    VmaAllocation allocation;
    vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);

    return std::make_unique<Buffer>(allocator, buffer, allocation);
}

std::unique_ptr<Buffer> Allocator::CreateImage(VkExtent2D extent, uint32_t p_buffer_usage, VmaMemoryUsage p_alloc_usage,
                                               uint32_t p_alloc_flag) {
    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_B8G8R8_UNORM;
    imageInfo.extent = {extent.width, extent.height, 1};
    imageInfo.usage = p_buffer_usage;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = p_alloc_usage;
    allocInfo.flags = p_alloc_flag;
    // allocInfo.memoryTypeBits = memory_;

    VkImage textureImage;
    VmaAllocation allocation;
    vmaCreateImage(allocator, &imageInfo, &allocInfo, &textureImage, &allocation, nullptr);

    return std::make_unique<Buffer>(allocator, textureImage, allocation);
}

void Allocator::Destroy() const {
    vmaDestroyAllocator(allocator);
}

} // end namespace lvk
