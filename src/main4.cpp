//
// Created by HotHat on 2025/10/23.
//

#include <iostream>
#define GLFW_INCLUDE_VULKAN
#include <fstream>
#include <GLFW/glfw3.h>

#include <vulkan_context.h>
#include <render_context.h>
#include <simple_draw.h>

// #include <buffer.h>

#include <glm/glm.hpp>
#include "allocator.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

struct Init {
    GLFWwindow *window;
    lvk::VulkanContext context{};
};

struct RenderData {
    VkQueue graphics_queue;
    VkQueue present_queue;

    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    std::vector<VkFramebuffer> framebuffers;

    VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;

    std::vector<VkSemaphore> available_semaphores;
    std::vector<VkSemaphore> finished_semaphore;
    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> image_in_flight;
    size_t current_frame = 0;
};

int device_initialization(Init &init) {
    init.window = glfwCreateWindow(800, 600, "Hello World!", nullptr, nullptr);

    // 1. Get the number of required instance extensions
    uint32_t count = 0;
    const char **extensions = glfwGetRequiredInstanceExtensions(&count);
    std::cout << "count: " << count << "\n";
    for (int i = 0; i < count; ++i) {
        std::cout << extensions[i] << "\n";
    }

    lvk::InstanceBuilder builder;
    init.context.instance = builder.SetAppName("Example Vulkan Application")
            .AddAvailableExtensions(count, extensions)
            .RequestValidationLayers()
            .EnableExtensions(count, extensions)
            .UseDefaultDebugMessenger()
            .Build();

    // lvk::InstanceBuilder instance_builder;
    // init.instance = instance_builder.UseDefaultDebugMessenger().RequestValidationLayers().Build();

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkResult glfw_result = glfwCreateWindowSurface(init.context.instance.instance, init.window, nullptr, &init.context.surface);
    if (glfw_result != VK_SUCCESS) {
        std::cerr << "Failed to select create window surface. Error: " << std::to_string(glfw_result) << "\n";
        return EXIT_FAILURE;
    }

    lvk::PhysicalDeviceSelector phys_device_selector(init.context.instance);

    auto physical_device = phys_device_selector.SetSurface(init.context.surface).Select();


    lvk::DeviceBuilder device_builder{physical_device};

    init.context.device = device_builder.Build();

    return 0;
}

int create_swapchain(lvk::VulkanContext &context) {
    lvk::SwapchainBuilder swapchain_builder{context.device};
    auto swapchain = swapchain_builder.SetOldSwapchain(context.swapchain).Build();

    lvk::destroy_swapchain(context.swapchain);

    context.swapchain = swapchain;
    return 0;
}

int get_queues(Init &init, RenderData &data) {
    // data.graphics_queue = init.device.GetQueue(lvk::QueueType::kGraphics);
    // data.present_queue = init.device.GetQueue(lvk::QueueType::kPresent);
    return 0;
}

int create_render_pass(Init &init, RenderData &data) {
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = init.context.swapchain.image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (vkCreateRenderPass(init.context.device.device, &render_pass_info, nullptr, &data.render_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass\n");
    }
    return 0;
}




void create_command_buffers_v2(lvk::RenderContext &ctx) {
    ctx.command_buffers.resize(ctx.framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = ctx.command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = ctx.command_buffers.size();

    if (vkAllocateCommandBuffers(ctx.context.device.device, &allocInfo, ctx.command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers");
    }

    for (size_t i = 0; i < ctx.command_buffers.size(); i++) {
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(ctx.command_buffers[i], &begin_info) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer");
        }

        VkRenderPassBeginInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = ctx.render_pass;
        render_pass_info.framebuffer = ctx.framebuffers[i];
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = ctx.context.swapchain.extent;
        VkClearValue clearColor{{{0.0f, 0.0f, 0.0f, 1.0f}}};
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clearColor;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(ctx.context.swapchain.extent.width);
        viewport.height = static_cast<float>(ctx.context.swapchain.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = ctx.context.swapchain.extent;

        vkCmdSetViewport(ctx.command_buffers[i], 0, 1, &viewport);
        vkCmdSetScissor(ctx.command_buffers[i], 0, 1, &scissor);

        vkCmdBeginRenderPass(ctx.command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(ctx.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.graphics_pipeline);

        vkCmdDraw(ctx.command_buffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(ctx.command_buffers[i]);

        if (vkEndCommandBuffer(ctx.command_buffers[i]) != VK_SUCCESS) {
            std::cout << "failed to record command buffer\n";
            throw std::runtime_error("failed to record command buffer");
        }
    }
}

int recreate_swapchain(lvk::RenderContext &ctx) {
    vkDeviceWaitIdle(ctx.context.device.device);

    vkDestroyCommandPool(ctx.context.device.device, ctx.command_pool, nullptr);

    for (auto framebuffer: ctx.framebuffers) {
        vkDestroyFramebuffer(ctx.context.device.device, framebuffer, nullptr);
    }

    ctx.context.swapchain.DestroyImageViews(ctx.swapchain_image_views);

    if (0 != create_swapchain(ctx.context)) return -1;
    // context.reset_swapchain(init.context.swapchain);
    ctx.create_framebuffers();
    ctx.create_command_pool();
    create_command_buffers_v2(ctx);

    // if (0 != create_framebuffers(init, data)) return -1;
    // if (0 != create_command_pool(init, data)) return -1;
    // if (0 != create_command_buffers(init, data)) return -1;
    return 0;
}

// int draw_frame(Init &init, RenderData &data) {
// int draw_frame(lvk::RenderContext &ctx) {
//     vkWaitForFences(ctx.context.device.device, 1, &ctx.in_flight_fences[ctx.current_frame], VK_TRUE, UINT64_MAX);
//
//     uint32_t image_index = 0;
//     VkResult result = vkAcquireNextImageKHR(ctx.context.device.device,
//                                             ctx.context.swapchain.swapchain, UINT64_MAX, ctx.available_semaphores[ctx.current_frame], VK_NULL_HANDLE, &image_index);
//
//     if (result == VK_ERROR_OUT_OF_DATE_KHR) {
//         return recreate_swapchain(ctx);
//     } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
//         std::cout << "failed to acquire swapchain image. Error " << result << "\n";
//         return -1;
//     }
//
//     if (ctx.image_in_flight[image_index] != VK_NULL_HANDLE) {
//         vkWaitForFences(ctx.context.device.device, 1, &ctx.image_in_flight[image_index], VK_TRUE, UINT64_MAX);
//     }
//     ctx.image_in_flight[image_index] = ctx.in_flight_fences[ctx.current_frame];
//
//     VkSubmitInfo submitInfo = {};
//     submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//     VkSemaphore wait_semaphores[] = {ctx.available_semaphores[ctx.current_frame]};
//     VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
//     submitInfo.waitSemaphoreCount = 1;
//     submitInfo.pWaitSemaphores = wait_semaphores;
//     submitInfo.pWaitDstStageMask = wait_stages;
//
//     submitInfo.commandBufferCount = 1;
//     submitInfo.pCommandBuffers = &ctx.command_buffers[image_index];
//
//     VkSemaphore signal_semaphores[] = {ctx.finished_semaphore[image_index]};
//     submitInfo.signalSemaphoreCount = 1;
//     submitInfo.pSignalSemaphores = signal_semaphores;
//
//     vkResetFences(ctx.context.device.device, 1, &ctx.in_flight_fences[ctx.current_frame]);
//
//     if (vkQueueSubmit(ctx.graphics_queue, 1, &submitInfo, ctx.in_flight_fences[ctx.current_frame]) !=
//         VK_SUCCESS) {
//         std::cout << "failed to submit draw command buffer\n";
//         return -1; //"failed to submit draw command buffer
//     }
//
//     VkPresentInfoKHR present_info = {};
//     present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//
//     present_info.waitSemaphoreCount = 1;
//     present_info.pWaitSemaphores = signal_semaphores;
//
//     VkSwapchainKHR swapChains[] = {ctx.context.swapchain.swapchain};
//     present_info.swapchainCount = 1;
//     present_info.pSwapchains = swapChains;
//
//     present_info.pImageIndices = &image_index;
//
//     result = vkQueuePresentKHR(ctx.present_queue, &present_info);
//     if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
//         return recreate_swapchain(ctx);
//     } else if (result != VK_SUCCESS) {
//         std::cout << "failed to present swapchain image\n";
//         return -1;
//     }
//
//     ctx.current_frame = (ctx.current_frame + 1) % ctx.max_frames_in_flight;
//     return 0;
// }

void cleanup(Init &init, lvk::RenderContext &context) {
    context.clearup();

    glfwDestroyWindow(init.window);
    glfwTerminate();
}

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    /*
    GLFWwindow *window = nullptr;
    window = glfwCreateWindow(800, 600, "Hello World!", nullptr, nullptr);

    auto info = lvk::SystemInfo::get_system_info();
    auto support = glfwVulkanSupported();
    std::cout << "support: " << support << "\n";

    uint32_t count;
    // 1. Get the number of required instance extensions
    const char **extensions = glfwGetRequiredInstanceExtensions(&count);
    std::cout << "count: " << count << "\n";
    for (int i = 0; i < count; ++i) {
        std::cout << extensions[i] << "\n";
    }

    lvk::InstanceBuilder builder;
    auto instance = builder.SetAppName("Example Vulkan Application")
            .AddAvailableExtensions(count, extensions)
            .RequestValidationLayers()
            .EnableExtensions(count, extensions)
            .UseDefaultDebugMessenger()
            .Build();

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult glfw_result = glfwCreateWindowSurface(instance.instance, window, nullptr, &surface);
    if (glfw_result != VK_SUCCESS) {
        std::cerr << "Failed to select create window surface. Error: " << std::to_string(glfw_result) << "\n";
        return EXIT_FAILURE;
    }

    lvk::PhysicalDeviceSelector selector{instance};
    auto physical_device = selector.SetSurface(surface).Select();

    std::cout << "Success to select physical device: " << physical_device.name << "\n";


    lvk::DeviceBuilder device_builder{physical_device};
    // automatically propagate needed data from instance & physical device
    auto lvk_device = device_builder.Build();

    std::cout << "Success to select logic device\n";

    // Get the VkDevice handle used in the rest of a vulkan application
    VkDevice device = lvk_device.device;

    // Get the graphics queue with a helper function
    auto graphics_queue = lvk_device.GetQueue(lvk::QueueType::kGraphics);
    auto graphics_queue_index = lvk_device.GetQueueIndex(lvk::QueueType::kGraphics);
    auto present_queue = lvk_device.GetQueue(lvk::QueueType::kPresent);
    auto present_queue_index = lvk_device.GetQueueIndex(lvk::QueueType::kPresent);
    std::cout << "Success to select graphics queue\n";
    std::cout << "Success to select graphics queue index:" << graphics_queue_index << "\n";
    std::cout << "Success to select present queue\n";
    std::cout << "Success to select present queue index:" << present_queue_index << "\n";


    lvk::SwapchainBuilder swapchain_builder{lvk_device};
    auto lvk_swapchain = swapchain_builder.Build();
    std::cout << "Success create swapchain\n";


    // lvk::destroy_swapchain(lvk_swapchain);
    // lvk::destroy_device(lvk_device);
    // lvk::destroy_surface(instance, surface);
    // lvk::destroy_instance(instance);
    //
    // glfwDestroyWindow(window);
    // glfwTerminate();
    */

    Init init;
    // RenderData render_data;
/*
    device_initialization(init);
    create_swapchain(init);
    get_queues(init, render_data);
    create_render_pass(init, render_data);
    create_graphics_pipeline(init, render_data);
    create_framebuffers(init, render_data);
    create_command_pool(init, render_data);
    create_command_buffers(init, render_data);
    create_sync_objects(init, render_data);
*/

    device_initialization(init);
    // create_swapchain(init.context);

    const std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint16_t> indices = {
            0, 1, 2, 2, 3, 0
    };
    uint32_t vertice_size =   sizeof(vertices[0]) * vertices.size();
    uint32_t indices_size =   sizeof(indices[0]) * indices.size();
    lvk::Allocator allocator(init.context);
    auto verticeBuffer = allocator.CreateBuffer(vertice_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    auto indicesBuffer = allocator.CreateBuffer(indices_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    verticeBuffer.CopyData(vertice_size, (void *) vertices.data());
    verticeBuffer.Flush();
    // vkb::allocated::init(init.context.device.device);
    // auto device_hpp = vk::Device(init.context.device.device);

    // auto device_resource = vkb::core::VulkanResource<vkb::BindingType::Cpp,  vk::Device>(device_hpp);
    // device_resource.set_debug_name("");


   // auto device = std::make_unique<vkb::Device>(init.context.device.physical_device,
   //                                       static_cast<VkSurfaceKHR>(init.context.surface),
   //                                             std::unique_ptr<vkb::DebugUtils>({}),
   //                                       lvk::SystemInfo::get_system_info().available_extensions);
    // vkb::core::BufferBuilderC buffer_builder(sizeof(vertices[0]) * vertices.size());
    // auto buffer = buffer_builder
    //         .with_vma_preferred_flags(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
    //         .with_usage(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            // .get_create_info();
            // .with_sharing_mode(VK_SHARING_MODE_EXCLUSIVE)
            // .build(device_resource);


    init.context.create_swapchain();

    lvk::SimpleDraw simple_draw(init.context);
    lvk::RenderContext render(init.context, simple_draw.render_pass, simple_draw.pipeline_layout, simple_draw.graphics_pipeline);

    // create_render_pass(init, render_data);
    // create_graphics_pipeline(init, render_data);

    // lvk::RenderContext context(init.context);

    // lvk::RenderContext context(init.context,
    //                            render_data.render_pass, render_data.pipeline_layout,
    //                            render_data.graphics_pipeline);


    // create_command_buffers_v2(render);

    while (!glfwWindowShouldClose(init.window)) {
        glfwPollEvents();
        // int res = draw_frame(render);
        // if (res != 0) {
        //     std::cout << "failed to draw frame \n";
        //     return -1;
        // }
        create_command_buffers_v2(render);

        render.rendering();
    }

    vkDeviceWaitIdle(init.context.device.device );

    verticeBuffer.Destroy();
    indicesBuffer.Destroy();
    
    allocator.Destroy();
    cleanup(init, render);

    return EXIT_SUCCESS;
}
