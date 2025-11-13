//
// Created by admin on 2025/11/13.
//

#ifndef LYH_BUFFER_H
#define LYH_BUFFER_H
#include <vulkan/vulkan.h>

namespace lvk {

struct Buffer{
    explicit Buffer(VmaAllocator &allocator_, VkBuffer buffer_, VmaAllocation allocation_):
            allocator(allocator_),
            buffer(buffer_),
            allocation(allocation_)
    {}

    void Destroy() const {
        // vmaUnmapMemory(allocator, allocation);
        vmaDestroyBuffer(allocator, buffer, allocation);
    }

    void CopyData(uint32_t size, void *data) {
        void* mappedData;
        vmaMapMemory(allocator, allocation, &mappedData);
        memcpy(mappedData, data, size);
        vmaUnmapMemory(allocator, allocation);
    }

    void Flush(VkDeviceSize offset, VkDeviceSize size) {
        vmaFlushAllocation(allocator, allocation, offset, size);
    }

    // ~Buffer() {
    //     VmaAllocation allocation = nullptr;
    //     vmaDestroyBuffer(allocator, buffer, allocation);
    // }

    VmaAllocator &allocator;
    VkBuffer buffer;
    VmaAllocation allocation;
};

} // end namespace lvk
#endif //LYH_BUFFER_H
