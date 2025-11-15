//
// Created by HotHat on 2025/10/23.
//

#include <iostream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan_context.h>
#include <render_context.h>
#include <simple_draw.h>

// #include <buffer.h>

#include <glm/glm.hpp>
#include "allocator.h"
#include "draw_model.h"


struct Init {
    GLFWwindow *window;
    lvk::VulkanContext context{};
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

    VkResult glfw_result = glfwCreateWindowSurface(init.context.instance.instance, init.window, nullptr,
                                                   &init.context.surface);
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
        viewport.height = static_cast<float>(ctx.context.swapchain.extent.height / 2);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent.width = ctx.context.swapchain.extent.width;
        scissor.extent.height = ctx.context.swapchain.extent.height / 2;

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


void create_command_buffers_v3(lvk::RenderContext &ctx, uint32_t image_index) {
    // VkCommandBufferAllocateInfo allocInfo = {};
    // allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    // allocInfo.commandPool = ctx.command_pool;
    // allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    // allocInfo.commandBufferCount = 1;

    // if (vkAllocateCommandBuffers(ctx.context.device.device, &allocInfo, &ctx.command_buffers[image_index]) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to allocate command buffers");
    // }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    auto commandBuffer = ctx.command_buffers[image_index];

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = ctx.render_pass;
    renderPassInfo.framebuffer = ctx.framebuffers[image_index];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = ctx.context.swapchain.extent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.graphics_pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(ctx.context.swapchain.extent.width);
    viewport.height = static_cast<float>(ctx.context.swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = ctx.context.swapchain.extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void cleanup(Init &init, lvk::RenderContext &context) {
    context.clearup();

    glfwDestroyWindow(init.window);
    glfwTerminate();
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


    Init init;

    device_initialization(init);

    init.context.create_swapchain();

    // lvk::SimpleDraw simple_draw(init.context);
    // lvk::RenderContext render(init.context, simple_draw.render_pass, simple_draw.pipeline_layout,
    // simple_draw.graphics_pipeline);


    VkRenderPass render_pass = init.context.CreateDefaultRenderPass();

    lvk::DrawModel model(init.context, render_pass);

    lvk::RenderContext render(init.context, model.render_pass);

    model.load();

    auto draw = [&model](lvk::RenderContext &render) {
        model.draw(render);
    };

    while (!glfwWindowShouldClose(init.window)) {
        glfwPollEvents();

        render.rendering(draw);

        // render.RenderBegin();
        // create_command_buffers_v2(render);
        // create_command_buffers_v3(render, render.image_index);
        // model.draw(render);

        // render.RenderEnd();
    }


    vkDeviceWaitIdle(init.context.device.device);

    model.destroy();

    cleanup(init, render);

    return EXIT_SUCCESS;
}
