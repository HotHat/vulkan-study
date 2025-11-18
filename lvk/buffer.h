//
// Created by admin on 2025/11/13.
//

#ifndef LYH_BUFFER_H
#define LYH_BUFFER_H
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace lvk {

struct Buffer{
    explicit Buffer(VmaAllocator &allocator_, VkBuffer buffer_, VmaAllocation allocation_):
            allocator(allocator_),
            buffer(buffer_),
            allocation(allocation_)
    {}

    // The Move Constructor
    Buffer(Buffer&& other) noexcept
        : allocator(other.allocator),
          buffer(other.buffer),
          allocation(other.allocation)
    {
    }
    // The Move Assignment Operator
    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            allocator = other.allocator;
            buffer = other.buffer;
            allocation = other.allocation;
        }

        return *this;
    }

    void Destroy() const {
        // vmaUnmapMemory(allocator, allocation);
        vmaDestroyBuffer(allocator, buffer, allocation);
    }

    void CopyData(uint32_t p_size, void *data) {
        void* mappedData;
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
    VkBuffer buffer;
    VmaAllocation allocation;
};

} // end namespace lvk
#endif //LYH_BUFFER_H
