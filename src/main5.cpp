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
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "draw_model.h"


struct Init {
    GLFWwindow *window;
    std::unique_ptr<lvk::VulkanContext> context{};
    lvk::GlobalUbo ubo{};
    bool is_resizing = false;
    bool framebufferResized = false;
    std::unique_ptr<lvk::DrawModel> model;
    std::unique_ptr<lvk::RenderContext> render;

    void UploadUbo(int width, int height) {
        auto view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        auto projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), -5.0f,
                                     5.0f);

        auto model = glm::mat4(1.0f);
        ubo.mvp = projection * view * model;
    }
    void ReSize(uint32_t width, uint32_t height) {
        render->ReSize(width, height);
        Render();
    }

    void Render() {
        render->RenderBegin();

        render->RenderPassBegin();
        // create_command_buffers_v2(render);
        // create_command_buffers_v3(render, render.image_index);
        model->UpdateUniform(ubo);
        model->Draw();
        // model2.draw(render);

        render->RenderPassEnd();
        render->RenderEnd();
        render->SetDebug(false);
    }

    void Cleanup() const {
        model->Destroy();
        render->Cleanup();
        context->Cleanup();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

Init init;

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
    init.UploadUbo(800, 600);

    //
    init.render = std::make_unique<lvk::RenderContext>(*init.context);
    init.model = std::make_unique<lvk::DrawModel>(*init.render);
    init.model->DrawRectangle({100.0f, 100.0f}, {100.0f, 100.0f}, {1.0f, 1.0f, 0.0f});
    init.model->DrawRectangle({250.0f, 100.0f}, {100.0f, 100.0f}, {0.0f, 1.0f, 1.0f});
    init.model->DrawRectangle({400.0f, 100.0f}, {100.0f, 100.0f}, {1.0f, 0.0f, 0.0f});

    init.model->AddDrawObject();
    init.model->DrawTriangle({150.0f, 250.0f}, {200.0f, 350.0f}, {100.0f, 350.0f}, {1.0f, 0.0f, 0.0f});
    init.model->DrawRectangle({250.0f, 350.0f}, {100.0f, 100.0f}, {0.0f, 1.0f, 1.0f});

    init.model->LoadVertex();
    //
    // init.model->LoadImage();

    return 0;
}

void resize(GLFWwindow *window, int width, int height) {
    init.UploadUbo(width, height);
    std::cout << "window resize =========> width: " << width << " height: " << height << std::endl;
    init.is_resizing = true;
    init.framebufferResized = true;
    // init.model->UpdateUniform(init.ubo);
}

void frame_resize(GLFWwindow *window, int width, int height) {
    init.framebufferResized = true;
    init.render->ReSize(width, height);
    init.UploadUbo(width, height);
    std::cout << "framebuffer resize ====> width: " << width << " height: " << height << std::endl;
    // init.render->SetDebug(true);
    // glfwGetFramebufferSize(window, &width, &height);
    // std::cout << "framebuffer resize2 ===> width: " << width << " height: " << height << std::endl;
    // init.render->RenderBegin();

    // init.render->RenderPassBegin();
    // create_command_buffers_v2(render);
    // create_command_buffers_v3(render, render.image_index);

    // init.model->UpdateUniform(init.ubo);
    // init.model->draw(*init.render);
    // model2.draw(render);

    // init.render->RenderPassEnd();
    // init.render->RenderEnd();

    //
    init.ReSize(width, height);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


    device_initialization(init);
    // glfwSetWindowSizeCallback(init.window, resize);
    glfwSetFramebufferSizeCallback(init.window, frame_resize);

    // init.context.CreateSwapchain();

    // lvk::SimpleDraw simple_draw(init.context);
    // lvk::RenderContext render(init.context, simple_draw.render_pass, simple_draw.pipeline_layout,
    // simple_draw.graphics_pipeline);


    // VkRenderPass render_pass = init.context->GetDefaultRenderPass();

    // lvk::RenderContext render(*init.context);

    // lvk::DrawModel model(*init.context);
    // model.load3();

    // lvk::DrawModel model2(*init.context);
    // model2.load2();


    // auto draw = [&model](lvk::RenderContext &render) {
    // model.draw(render);
    // };
    // model.UpdateUniform(init.ubo);

    while (!glfwWindowShouldClose(init.window)) {
        glfwPollEvents();

        // if (!init.is_resizing) {
        // std::cout << "after resizing!" << std::endl;
        // }

        // if (init.is_resizing) {
        // render.RecreateSwapchain();
        // init.is_resizing = false;
        // std::cout << "resizing updated!" << std::endl;
        // init.model->UpdateUniform(init.ubo);
        // continue;
        // }
        // init.model->UpdateUniform(init.ubo);
        // render.rendering(draw);

        // auto command_buffer = render.BeginSingleTimeCommands();
        // model.UpdateUniform2(command_buffer, init.ubo);
        // render.EndSingleTimeCommands(command_buffer);

        init.Render();
    }


    vkDeviceWaitIdle(init.context->device.device);

    init.Cleanup();
    // cleanup(init);

    return EXIT_SUCCESS;
}
