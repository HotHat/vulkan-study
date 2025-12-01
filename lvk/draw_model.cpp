//
// Created by admin on 2025/11/14.
//

#include "draw_model.h"

#include <array>
#include <ostream>
#include <vector>
#include "functions.h"
#include <glm/gtc/matrix_transform.hpp>
#include <variant>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "pipeline_manager.h"

namespace lvk {
DrawModel::DrawModel(RenderContext &context) : context(context) {
    // create_render_pass();
    // render_pass = context.GetContext().GetDefaultRenderPass();
    // allocator = std::make_unique<Allocator>(context.GetContext());

    // createDescriptorSet();
    auto setLayout = DescriptorSetLayout::Builder(context.GetContext().device)
                   .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                   // .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                   .Build();

    auto pipeline_ = Pipeline::Builder(context.GetContext())
            .withShader("../shaders/ubo.vert.spv", "../shaders/ubo.frag.spv")
            .withBindingDescription({
                .binding = 0,
                .stride = sizeof(Vertex2),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            })
            .withVertexDescriptions(Vertex2::GetAttributeDescriptions())
            .withDescriptorSetLayout(setLayout)
            .build();

    auto &manage = PipelineManage::Instance();
    manage.Add("base_shader", std::make_unique<Pipeline>(std::move(pipeline_)));

    // shader 2
    auto setLayout2 = DescriptorSetLayout::Builder(context.GetContext().device)
                .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .Build();

    auto pipeline2 = Pipeline::Builder(context.GetContext())
            .withShader("../shaders/textures.vert.spv", "../shaders/textures.frag.spv")
            .withBindingDescription({
                .binding = 0,
                .stride = sizeof(Vertex3),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            })
            .withVertexDescriptions(Vertex3::GetAttributeDescriptions())
            .withDescriptorSetLayout(setLayout2)
            .build();

    manage.Add("image_shader", std::make_unique<Pipeline>(std::move(pipeline2)));

    AddDrawObject();

    // draw_objects.emplace_back();

    // vertex_buffers = allocator->CreateBuffer(65535,
    //                                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    //                                         VMA_MEMORY_USAGE_CPU_TO_GPU);
    // indices_buffers = allocator->CreateBuffer(65535,
    //                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    //                                          VMA_MEMORY_USAGE_CPU_TO_GPU);
    //
    // ubo_buffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
    //
    // for (auto &ubo_buffer: ubo_buffers) {
    //     ubo_buffer = allocator->CreateBuffer2(sizeof(GlobalUbo),
    //                                           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    //                                           VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    //                                           VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
    //                                           VMA_ALLOCATION_CREATE_MAPPED_BIT
    //     );
    // }
    //
    // LoadImage();
    // load3();
    // createDescriptorSet();
    // CreateGraphicsPipeline2();
    // CreateGraphicsPipeline3("../shaders/textures.vert.spv", "../shaders/textures.frag.spv");
}

void DrawModel::AddDrawObject() {
    // CreateGraphicsPipeline2();
    // auto setLayout = DescriptorSetLayout::Builder(context.GetContext().device)
    //             .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
    //             // .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
    //             .Build();
    //
    // auto pipeline_ = Pipeline::Builder(context.GetContext())
    //         .withShader("../shaders/ubo.vert.spv", "../shaders/ubo.frag.spv")
    //         .withBindingDescription({
    //             .binding = 0,
    //             .stride = sizeof(Vertex2),
    //             .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    //         })
    //         .withVertexDescriptions(Vertex2::GetAttributeDescriptions())
    //         .withDescriptorSetLayout(setLayout)
    //         .build();

    auto shadeId  = PipelineManage::Instance().Get("base_shader");
    auto draw_object = BaseDrawObject(shadeId);
    // auto obj = static_cast<DrawObjectVector2>(draw_object);
    // draw_object.WithPipeline(pipeline_);
            // .WithPipelineLayout(pipeline_layout);

    draw_objects.emplace_back(std::make_unique<BaseDrawObject>(std::move(draw_object)));
}

void DrawModel::AddDrawTextureObject(const std::string &image_path) {
    // CreateGraphicsPipeline3("../shaders/textures.vert.spv", "../shaders/textures.frag.spv");

    auto texture = std::make_unique<Texture>(context);
    // texture->LoadImage("textures/texture.jpg");
    texture->LoadImage(image_path);


    // auto draw_object = std::move(BaseDrawObject(pipeline_).WithTexture(texture));

    auto shadeId  = PipelineManage::Instance().Get("image_shader");
    auto draw_object = std::make_unique<BaseDrawObject>(shadeId);
    draw_object->WithTexture(texture);

    draw_objects.emplace_back(std::move(draw_object));
}

void DrawModel::LoadVertex() {
    ubo_buffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
    for (auto &ubo_buffer: ubo_buffers) {
        ubo_buffer = context.GetAllocator().CreateBuffer2(sizeof(GlobalUbo),
                                                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                          VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                                                          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                          VMA_ALLOCATION_CREATE_MAPPED_BIT
        );
    }

    int32_t index = 0;
    uint32_t uniformSize = 0;
    uint32_t imageSampleSize = 0;

    for (auto const &object: draw_objects) {
        auto &layout = object->GetDescriptorSetLayout();
        for (auto &binding: layout.getBindings()) {
            if (binding.second.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                uniformSize += 1;
            }
            if (binding.second.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                imageSampleSize += 1;
            }
        }
    }

    descriptorPool =
           DescriptorPool::Builder(context.GetContext().device)
           .SetMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT * (uniformSize + imageSampleSize))
           .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT * uniformSize)
           .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Swapchain::MAX_FRAMES_IN_FLIGHT * imageSampleSize)
           .Build();

    for (auto const &object: draw_objects) {
        uint32_t vertex_size = object->GetVertexDataSize();
        uint32_t indices_size = object->GetIndicesDataSize();

        vertex_buffers[index] = context.GetAllocator().CreateBuffer(vertex_size,
                                                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                    VMA_MEMORY_USAGE_CPU_TO_GPU);
        indices_buffers[index] = context.GetAllocator().CreateBuffer(indices_size,
                                                                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                     VMA_MEMORY_USAGE_CPU_TO_GPU);

        //
        vertex_buffers[index]->CopyData(vertex_size, const_cast<void *>(object->GetVertexData()));
        vertex_buffers[index]->Flush(0, vertex_size);

        indices_buffers[index]->CopyData(indices_size, const_cast<void *>(object->GetIndicesData()));
        indices_buffers[index]->Flush(0, indices_size);

        descriptor_sets[index].resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < Swapchain::MAX_FRAMES_IN_FLIGHT; i++) {
            auto bufferInfo = VkDescriptorBufferInfo{
                ubo_buffers[i]->buffer,
                0,
                VK_WHOLE_SIZE
                // ubo_buffers[i]->size,
                // range
            };

            auto writer = DescriptorWriter(object->GetDescriptorSetLayout(), *descriptorPool)
                    .WriteBuffer(0, &bufferInfo);

            if (object->HasTexture()) {
                auto &texture = object->GetTexture();
                auto textureInfo = VkDescriptorImageInfo{
                    texture.GetSampler(),
                    texture.GetImageView(),
                };

                textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                writer.WriteImage(1, &textureInfo);
            }
            writer.Build(descriptor_sets[index][i]);
        }

        auto &layout = object->GetDescriptorSetLayout();
        for (auto &binding: layout.getBindings()) {
            if (binding.second.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                uniformSize += 1;
            }
            if (binding.second.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                imageSampleSize += 1;
            }
        }

        index++;
    }


}


void DrawModel::Destroy() {
    for (auto const &buffer: vertex_buffers) {
        buffer.second->Destroy();
    }
    for (auto const &buffer: indices_buffers) {
        buffer.second->Destroy();
    }
    // texture->Destroy();

    for (int i = 0; i < ubo_buffers.size(); i++) {
        ubo_buffers[i]->Destroy();
    }


    descriptorPool->Cleanup();
    // descriptorSetLayout->Cleanup();

    // vkDeviceWaitIdle(context.GetContext().device.device);
    for (auto const &object: draw_objects) {
        object->Cleanup();
        // object->GetDescriptorSetLayout().Cleanup();
        // vkDestroyPipeline(context.GetContext().device.device, object->GetPipeline(), nullptr);
        // vkDestroyPipelineLayout(context.GetContext().device.device, object->GetPipelineLayout(), nullptr);
    }
}

void DrawModel::DrawTriangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec3 color) {
    auto &top = draw_objects.back();

    // auto &object = dynamic_cast<DrawObjectVector2 &>(*top);

    top->AddTriangle<Vertex2>(
        Vertex2(vec3(p1.x, p1.y, 0.0f), color),
        Vertex2(vec3(p2.x, p2.y, 0.0f), color),
        Vertex2(vec3(p3.x, p3.y, 0.0f), color)
    );
}

void DrawModel::DrawRectangle(glm::vec2 pos, glm::vec2 size, glm::vec3 color) {
    auto &top = draw_objects.back();

    top->AddRectangle<Vertex2>(
        Vertex2(vec3(pos.x, pos.y, 0.0f), color),
        Vertex2(vec3(pos.x + size.x, pos.y, 0.0f), color),
        Vertex2(vec3(pos.x + size.x, pos.y + size.y, 0.0f), color),
        Vertex2(vec3(pos.x, pos.y + size.y, 0.0f), color)
    );
}

void DrawModel::DrawRectangleUv(glm::vec2 pos, glm::vec2 size, glm::vec3 color) {
    auto &top = draw_objects.back();

    // auto &object = reinterpret_cast<DrawObjectVector3 &>(top);

    top->AddRectangle<Vertex3>(
        Vertex3(vec3(pos.x, pos.y, 0.0f), color, {1.0f, 0.0f}),
        Vertex3(vec3(pos.x + size.x, pos.y, 0.0f), color, {0.0f, 0.0f}),
        Vertex3(vec3(pos.x + size.x, pos.y + size.y, 0.0f), color, {0.0f, 1.0f}),
        Vertex3(vec3(pos.x, pos.y + size.y, 0.0f), color, {1.0f, 1.0f})
    );
}

void DrawModel::Draw() {
    assert(!draw_objects.empty() && "without draw objects");
    assert(!vertex_buffers.empty() && "init vertex buffer first");

    // auto current_frame_ = context.image_index;
    auto current_image_index = context.GetCurrentImageIndex();
    auto commandBuffer = context.GetCurrentCommandBuffer();

    auto index = 0;
    for (auto const &object: draw_objects) {
        VkBuffer vertexBuffers[] = {vertex_buffers[index]->buffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object->GetPipeline());

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, indices_buffers[index]->buffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object->GetPipelineLayout(), 0, 1,
                                &descriptor_sets[index][current_image_index], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(object->GetIndicesSize()), 1, 0, 0, 0);

        index++;
    }
}

void DrawModel::UpdateUniform(GlobalUbo &ubo) {
    for (int i = 0; i < ubo_buffers.size(); i++) {
        ubo_buffers[i]->CopyData(sizeof(GlobalUbo), (void *) &ubo);
        ubo_buffers[i]->Flush();
    }
}

void DrawModel::UpdateUniform2(VkCommandBuffer command_buffer, GlobalUbo &ubo) {
    for (int i = 0; i < ubo_buffers.size(); i++) {
        ubo_buffers[i]->CopyData(sizeof(GlobalUbo), (void *) &ubo);
        ubo_buffers[i]->Flush();

        VkBufferMemoryBarrier bufMemBarrier = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
        bufMemBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        bufMemBarrier.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
        bufMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufMemBarrier.buffer = ubo_buffers[i]->buffer;
        bufMemBarrier.offset = 0;
        bufMemBarrier.size = VK_WHOLE_SIZE;

        // It's important to insert a buffer memory barrier here to ensure writing to the buffer has finished.
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                             0, 0, nullptr, 1, &bufMemBarrier, 0, nullptr);
    }
}


void DrawModel::createDescriptorSet() {
    descriptorPool =
            DescriptorPool::Builder(context.GetContext().device)
            .SetMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT * 6)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT * 3)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Swapchain::MAX_FRAMES_IN_FLIGHT * 3)
            .Build();

    // descriptorSetLayout =
            // DescriptorSetLayout::Builder(context.GetContext().device)
            // .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            // .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            // .Build();

    // descriptorSets.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
}
} // end namespace lvk
