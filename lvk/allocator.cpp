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

Buffer Allocator::CreateBuffer(VkDeviceSize size_, uint32_t usage_, VmaMemoryUsage memory_) {
    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size_;
    bufferInfo.usage = usage_;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memory_;
    // allocInfo.memoryTypeBits = memory_;

    VkBuffer buffer;
    VmaAllocation allocation;
    vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);

    return Buffer(allocator, buffer, allocation);
}

void Allocator::Destroy() {
    vmaDestroyAllocator(allocator);
}

} // end namespace lvk
