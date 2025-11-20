//
// Created by admin on 2025/11/13.
//

#ifndef LYH_ALLOCATOR_H
#define LYH_ALLOCATOR_H
#include <vma/vk_mem_alloc.h>

#include "vulkan_context.h"
#include "buffer.h"
#include "image.h"

namespace lvk {

class Allocator {
public:
    explicit Allocator(VulkanContext &context_);
    void Destroy() const;
    ~Allocator();

    std::unique_ptr<Buffer> CreateBuffer(VkDeviceSize size_, uint32_t usage_, VmaMemoryUsage memory_);
    std::unique_ptr<Buffer> CreateBuffer2(VkDeviceSize p_buffer_size, uint32_t p_buffer_usage, VmaMemoryUsage p_alloc_usage, uint32_t p_alloc_flag);
    std::unique_ptr<Image> CreateImage(VkExtent2D extent, VkFormat format, VkImageTiling tiling, uint32_t p_buffer_usage, VmaMemoryUsage p_alloc_usage, uint32_t p_alloc_flag);

private:
    VmaAllocator allocator{};
    VulkanContext &context;
};


} // end namespace lvk

#endif //LYH_ALLOCATOR_H
