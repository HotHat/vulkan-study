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
    std::unique_ptr<lvk::VulkanContext> context{};

    void Cleanup() const {
        context->Cleanup();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
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
    auto instance = builder.SetAppName("Example Vulkan Application")
            .AddAvailableExtensions(count, extensions)
            .RequestValidationLayers()
            .EnableExtensions(count, extensions)
            .UseDefaultDebugMessenger()
            .Build();

    // lvk::InstanceBuilder instance_builder;
    // init.instance = instance_builder.UseDefaultDebugMessenger().RequestValidationLayers().Build();

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkResult glfw_result = glfwCreateWindowSurface(instance.instance, init.window, nullptr,
                                                   &surface);
    if (glfw_result != VK_SUCCESS) {
        std::cerr << "Failed to select create window surface. Error: " << std::to_string(glfw_result) << "\n";
        return EXIT_FAILURE;
    }

    lvk::PhysicalDeviceSelector phys_device_selector(instance);

    auto physical_device = phys_device_selector.SetSurface(surface).Select();


    lvk::DeviceBuilder device_builder{physical_device};

    auto device = device_builder.Build();

    //
    init.context = std::make_unique<lvk::VulkanContext>(init.window, instance, surface, device);

    return 0;
}

int create_swapchain(lvk::VulkanContext &context) {
    lvk::SwapchainBuilder swapchain_builder{context.device};
    auto swapchain = swapchain_builder.SetOldSwapchain(context.swapchain).Build();

    lvk::destroy_swapchain(context.swapchain);

    context.swapchain = swapchain;
    return 0;
}


void cleanup(Init &init, lvk::RenderContext &context) {
    context.Cleanup();
    init.Cleanup();
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


    Init init;

    device_initialization(init);

    // init.context.CreateSwapchain();

    // lvk::SimpleDraw simple_draw(init.context);
    // lvk::RenderContext render(init.context, simple_draw.render_pass, simple_draw.pipeline_layout,
    // simple_draw.graphics_pipeline);


    // VkRenderPass render_pass = init.context->GetDefaultRenderPass();

    lvk::RenderContext render(*init.context);

    lvk::DrawModel model(*init.context);
    // model.load3();

    // lvk::DrawModel model2(*init.context);
    // model2.load2();


    // auto draw = [&model](lvk::RenderContext &render) {
        // model.draw(render);
    // };

    while (!glfwWindowShouldClose(init.window)) {
        glfwPollEvents();

        // render.rendering(draw);

        render.RenderBegin();
        render.RenderPassBegin();
        // create_command_buffers_v2(render);
        // create_command_buffers_v3(render, render.image_index);
        model.draw(render);
        // model2.draw(render);

        render.RenderPassEnd();
        render.RenderEnd();
    }


    vkDeviceWaitIdle(init.context->device.device);

    model.destroy();

    cleanup(init, render);

    return EXIT_SUCCESS;
}
