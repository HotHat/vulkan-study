//
// Created by admin on 2025/11/13.
//

#ifndef LYH_ALLOCATOR_H
#define LYH_ALLOCATOR_H
#include <vma/vk_mem_alloc.h>

#include "vulkan_context.h"
#include "buffer.h"

namespace lvk {

class Allocator {
public:
    explicit Allocator(VulkanContext &context_);
    void Destroy();
    ~Allocator();

    Buffer CreateBuffer(VkDeviceSize size_, uint32_t usage_, VmaMemoryUsage memory_);

private:
    VmaAllocator allocator{};
};


} // end namespace lvk

#endif //LYH_ALLOCATOR_H
