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
    explicit Image(VmaAllocator &allocator_, VkImage image_, VmaAllocation allocation_): allocator(allocator_),
        image(image_),
        allocation(allocation_) {
    }

    void Destroy() const {
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
    VmaAllocator &allocator;
    VkImage image;
    VmaAllocation allocation;

};
} // end namespace lvk
#endif //LYH_IMAGE_H
