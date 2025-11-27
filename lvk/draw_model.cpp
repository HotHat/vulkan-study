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

namespace lvk {
DrawModel::DrawModel(RenderContext &context) : context(context) {
    // create_render_pass();
    render_pass = context.GetContext().GetDefaultRenderPass();
    // allocator = std::make_unique<Allocator>(context.GetContext());

    createDescriptorSet();
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

void DrawModel::load2() {
    // vertices = {
    //     {{-0.0f, -0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    //     {{0.5f, -0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    //     {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    //     {{-0.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    // };
    //
    // indices = {0, 1, 2, 2, 3, 0};
    //
    // uint32_t vertice_size = sizeof(vertices[0]) * vertices.size();
    // uint32_t indices_size = sizeof(indices[0]) * indices.size();
    //
    // vertex_buffer = allocator->CreateBuffer(vertice_size,
    //                                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    //                                         VMA_MEMORY_USAGE_CPU_TO_GPU);
    // indices_buffer = allocator->CreateBuffer(indices_size,
    //                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    //                                          VMA_MEMORY_USAGE_CPU_TO_GPU);
    //
    //
    // vertex_buffer->CopyData(vertice_size, (void *) vertices.data());
    // vertex_buffer->Flush(0, vertice_size);
    //
    // indices_buffer->CopyData(indices_size, (void *) indices.data());
    // indices_buffer->Flush(0, indices_size);
}

void DrawModel::AddDrawObject() {
    // CreateGraphicsPipeline2();
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

    auto draw_object = BaseDrawObject(pipeline_);
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

    auto setLayout = DescriptorSetLayout::Builder(context.GetContext().device)
                .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .Build();

    auto pipeline_ = Pipeline::Builder(context.GetContext())
            .withShader("../shaders/textures.vert.spv", "../shaders/textures.frag.spv")
            .withBindingDescription({
                .binding = 0,
                .stride = sizeof(Vertex3),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            })
            .withVertexDescriptions(Vertex3::GetAttributeDescriptions())
            .withDescriptorSetLayout(setLayout)
            .build();

    auto draw_object = std::move(BaseDrawObject(pipeline_).WithTexture(texture));

    draw_objects.emplace_back(std::make_unique<BaseDrawObject>(std::move(draw_object)));
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
    for (auto const &object: draw_objects) {
        uint32_t vertice_size = object->GetVertexDataSize();
        uint32_t indices_size = object->GetIndicesDataSize();

        vertex_buffers[index] = context.GetAllocator().CreateBuffer(vertice_size,
                                                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                    VMA_MEMORY_USAGE_CPU_TO_GPU);
        indices_buffers[index] = context.GetAllocator().CreateBuffer(indices_size,
                                                                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                     VMA_MEMORY_USAGE_CPU_TO_GPU);

        //
        vertex_buffers[index]->CopyData(vertice_size, const_cast<void *>(object->GetVertexData()));
        vertex_buffers[index]->Flush(0, vertice_size);

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

        index++;

        /*
        auto vi = obj.index();

        if (vi == 0) {
            auto const &object = std::get<DrawObject<Vertex2> >(obj);
            uint32_t vertice_size = sizeof(object.GetVertexes()[0]) * object.GetVertexes().size();
            uint32_t indices_size = sizeof(object.GetIndices()[0]) * object.GetIndices().size();

            vertex_buffers[index] = context.GetAllocator().CreateBuffer(vertice_size,
                                                                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                        VMA_MEMORY_USAGE_CPU_TO_GPU);
            indices_buffers[index] = context.GetAllocator().CreateBuffer(indices_size,
                                                                         VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                                         VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                         VMA_MEMORY_USAGE_CPU_TO_GPU);

            //
            vertex_buffers[index]->CopyData(vertice_size, (void *) object.GetVertexes().data());
            vertex_buffers[index]->Flush(0, vertice_size);

            indices_buffers[index]->CopyData(indices_size, (void *) object.GetIndices().data());
            indices_buffers[index]->Flush(0, indices_size);

            descriptor_sets[vi].resize(descriptorSets.size());
            for (int i = 0; i < descriptorSets.size(); i++) {
                auto bufferInfo = VkDescriptorBufferInfo{
                    ubo_buffers[i]->buffer,
                    0,
                    VK_WHOLE_SIZE
                    // ubo_buffers[i]->size,
                    // range
                };
                DescriptorWriter(*descriptorSetLayout, *descriptorPool)
                        .WriteBuffer(0, &bufferInfo)
                        .Build(descriptor_sets[vi][i]);
                // if (vi == 1) {
                //     auto &texture = object.GetTexture();
                //     auto textureInfo = VkDescriptorImageInfo{
                //         texture.GetSampler(),
                //         texture.GetImageView(),
                //     };
                //
                //     textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                //     writer.WriteImage(1, &textureInfo);
                // }

                // writer.Build(descriptor_sets[vi][i]);

                // DescriptorWriter(*descriptorSetLayout, *descriptorPool)
                // .WriteBuffer(0, &bufferInfo)
                // .Build(descriptorSets[i]);
            }
        } else {
            auto const &object = std::get<DrawObject<Vertex3> >(obj);
            uint32_t vertice_size = sizeof(object.GetVertexes()[0]) * object.GetVertexes().size();
            uint32_t indices_size = sizeof(object.GetIndices()[0]) * object.GetIndices().size();

            vertex_buffers[index] = context.GetAllocator().CreateBuffer(vertice_size,
                                                                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                        VMA_MEMORY_USAGE_CPU_TO_GPU);
            indices_buffers[index] = context.GetAllocator().CreateBuffer(indices_size,
                                                                         VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                                         VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                         VMA_MEMORY_USAGE_CPU_TO_GPU);

            //
            vertex_buffers[index]->CopyData(vertice_size, (void *) object.GetVertexes().data());
            vertex_buffers[index]->Flush(0, vertice_size);

            indices_buffers[index]->CopyData(indices_size, (void *) object.GetIndices().data());
            indices_buffers[index]->Flush(0, indices_size);

            descriptor_sets[vi].resize(descriptorSets.size());
            for (int i = 0; i < descriptorSets.size(); i++) {
                auto bufferInfo = VkDescriptorBufferInfo{
                    ubo_buffers[i]->buffer,
                    0,
                    VK_WHOLE_SIZE
                    // ubo_buffers[i]->size,
                    // range
                };
                auto &texture = object.GetTexture();
                auto textureInfo = VkDescriptorImageInfo{
                    texture.GetSampler(),
                    texture.GetImageView(),
                };

                textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                DescriptorWriter(*descriptorSetLayout, *descriptorPool)
                        .WriteBuffer(0, &bufferInfo)
                        .WriteImage(1, &textureInfo)
                        .Build(descriptor_sets[vi][i]);

                // writer.WriteImage(1, &textureInfo);

                // writer.Build(descriptor_sets[vi][i]);

                // DescriptorWriter(*descriptorSetLayout, *descriptorPool)
                // .WriteBuffer(0, &bufferInfo)
                // .Build(descriptorSets[i]);
            }
        }
        //


        descriptor_sets[vi].resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
        if (vi == 0) {
            for (int i = 0; i < descriptorSets.size(); i++) {
                auto bufferInfo = VkDescriptorBufferInfo{
                    ubo_buffers[i]->buffer,
                    0,
                    VK_WHOLE_SIZE
                    // ubo_buffers[i]->size,
                    // range
                };

                // auto textureInfo = VkDescriptorImageInfo{
                //     texture->sampler,
                //     texture->imageView,
                // };
                // textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


                DescriptorWriter(*descriptorSetLayout, *descriptorPool)
                        .WriteBuffer(0, &bufferInfo)
                        // .WriteImage(1, &textureInfo)
                        .Build(descriptorSets[i]);
            }
        } else {
        }
        */
    }


    // vertices = {
    //     // {{-0.0f, -0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    //     // {{0.5f, -0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    //     // {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    //     // {{-0.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
    //     {{100.0f, 100.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    //     {{200.0f, 100.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    //     {{200.0f, 200.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    //     {{100.0f, 200.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}
    // };
    //
    // indices = {0, 1, 2, 2, 3, 0};
    // assert(!indices.empty() && "indices is empty");
    //
    // uint32_t vertice_size = sizeof(vertices[0]) * vertices.size();
    // uint32_t indices_size = sizeof(indices[0]) * indices.size();
    //
    //
    // vertex_buffer->CopyData(vertice_size, (void *) vertices.data());
    // vertex_buffer->Flush(0, vertice_size);
    //
    // indices_buffer->CopyData(indices_size, (void *) indices.data());
    // indices_buffer->Flush(0, indices_size);


    // auto extent = context.swapchain.extent;
    // auto view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // auto projection = glm::ortho(0.0f, static_cast<float>(extent.width), 0.0f, static_cast<float>(extent.height), -5.0f,
    // 5.0f);


    // for (int i = 0; i < ubo_buffers.size(); i++) {


    // ubo_buffers[i]->CopyData(sizeof(GlobalUbo), (void *) &globalUbo);
    // ubo_buffers[i]->Flush();
    // }
    // UpdateUniform(globalUbo);
}

void DrawModel::LoadImage() {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    // mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    auto texture_buffer = context.GetAllocator().CreateBuffer2(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                               VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                                                               VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                               VMA_ALLOCATION_CREATE_MAPPED_BIT);
    texture_buffer->CopyData(imageSize, (void *) pixels);
    texture_buffer->Flush(0, imageSize);
    //
    stbi_image_free(pixels);

    // texture = allocator->CreateImage(
    //     {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)},
    //     VK_FORMAT_R8G8B8A8_SRGB,
    //     VK_IMAGE_TILING_OPTIMAL,
    //     VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    //     VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    //     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
    // );
    //
    //
    // TransitionImageLayout(texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
    //                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    //
    // CopyBufferToImage(texture_buffer->buffer, texture->image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    //
    // TransitionImageLayout(texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //
    // texture_buffer->Destroy();
    // texture->Destroy();
}

void DrawModel::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = context.BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    context.EndSingleTimeCommands(commandBuffer);
}

void DrawModel::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                                      VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = context.BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout ==
               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    context.EndSingleTimeCommands(commandBuffer);
}


void DrawModel::load() {
    // vertices = {
    //     {{-0.0f, -0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    //     {{0.5f, -0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    //     {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    //     {{-0.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
    // };
    //
    // indices = {0, 1, 2, 2, 3, 0};
    //
    // uint32_t vertice_size = sizeof(vertices[0]) * vertices.size();
    // uint32_t indices_size = sizeof(indices[0]) * indices.size();
    //
    // vertex_buffer = allocator->CreateBuffer(vertice_size,
    //                                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    //                                         VMA_MEMORY_USAGE_CPU_TO_GPU);
    // indices_buffer = allocator->CreateBuffer(indices_size,
    //                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    //                                          VMA_MEMORY_USAGE_CPU_TO_GPU);
    //
    //
    // vertex_buffer->CopyData(vertice_size, (void *) vertices.data());
    // vertex_buffer->Flush(0, vertice_size);
    //
    // indices_buffer->CopyData(indices_size, (void *) indices.data());
    // indices_buffer->Flush(0, indices_size);
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
    descriptorSetLayout->Cleanup();

    vkDeviceWaitIdle(context.GetContext().device.device);
    for (auto const &object: draw_objects) {
        object->Cleanup();
        vkDestroyPipeline(context.GetContext().device.device, object->GetPipeline(), nullptr);
        vkDestroyPipelineLayout(context.GetContext().device.device, object->GetPipelineLayout(), nullptr);
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

void DrawModel::CreateGraphicsPipeline() {
    auto vert_code = ReadFile("../shaders/vertbuffer.vert.spv");
    auto frag_code = ReadFile("../shaders/vertbuffer.frag.spv");

    VkShaderModule vert_module = CreateShaderModule(context.GetContext().device.device, vert_code);
    VkShaderModule frag_module = CreateShaderModule(context.GetContext().device.device, frag_code);
    if (vert_module == VK_NULL_HANDLE || frag_module == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to create shader module\n");
    }

    VkPipelineShaderStageCreateInfo vert_stage_info = {};
    vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage_info.module = vert_module;
    vert_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_stage_info = {};
    frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage_info.module = frag_module;
    frag_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage_info, frag_stage_info};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex2);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex2, pos);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex2, color);

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.vertexAttributeDescriptionCount = 2;
    vertex_input_info.pVertexBindingDescriptions = &bindingDescription;
    vertex_input_info.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(context.GetContext().swapchain.extent.width);
    viewport.height = static_cast<float>(context.GetContext().swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = context.GetContext().swapchain.extent;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &colorBlendAttachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(context.GetContext().device.device, &pipeline_layout_info, nullptr, &pipeline_layout)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout\n");
    }

    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_info = {};
    dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_info.pDynamicStates = dynamic_states.data();

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_info;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(context.GetContext().device.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
                                  &graphics_pipeline) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create pipline\n");
    }

    vkDestroyShaderModule(context.GetContext().device.device, frag_module, nullptr);
    vkDestroyShaderModule(context.GetContext().device.device, vert_module, nullptr);
}

void DrawModel::CreateGraphicsPipeline2() {
    auto vert_code = ReadFile("../shaders/ubo.vert.spv");
    auto frag_code = ReadFile("../shaders/ubo.frag.spv");

    VkShaderModule vert_module = CreateShaderModule(context.GetContext().device.device, vert_code);
    VkShaderModule frag_module = CreateShaderModule(context.GetContext().device.device, frag_code);
    if (vert_module == VK_NULL_HANDLE || frag_module == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to create shader module\n");
    }

    VkPipelineShaderStageCreateInfo vert_stage_info = {};
    vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage_info.module = vert_module;
    vert_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_stage_info = {};
    frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage_info.module = frag_module;
    frag_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage_info, frag_stage_info};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex2);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    auto attributeDescriptions = Vertex2::GetAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertex_input_info.pVertexBindingDescriptions = &bindingDescription;
    vertex_input_info.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(context.GetContext().swapchain.extent.width);
    viewport.height = static_cast<float>(context.GetContext().swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = context.GetContext().swapchain.extent;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &colorBlendAttachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    auto descriptor_set_Layout = descriptorSetLayout->getDescriptorSetLayout();
    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_set_Layout;
    // pipeline_layout_info.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(context.GetContext().device.device, &pipeline_layout_info, nullptr, &pipeline_layout)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout\n");
    }

    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_info = {};
    dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_info.pDynamicStates = dynamic_states.data();

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_info;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(context.GetContext().device.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
                                  &graphics_pipeline) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create pipline\n");
    }

    vkDestroyShaderModule(context.GetContext().device.device, frag_module, nullptr);
    vkDestroyShaderModule(context.GetContext().device.device, vert_module, nullptr);
}


void DrawModel::CreateGraphicsPipeline3(const std::string &vert_file, const std::string &frag_file) {
    auto vert_code = ReadFile(vert_file);
    auto frag_code = ReadFile(frag_file);

    VkShaderModule vert_module = CreateShaderModule(context.GetContext().device.device, vert_code);
    VkShaderModule frag_module = CreateShaderModule(context.GetContext().device.device, frag_code);
    if (vert_module == VK_NULL_HANDLE || frag_module == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to create shader module\n");
    }

    VkPipelineShaderStageCreateInfo vert_stage_info = {};
    vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage_info.module = vert_module;
    vert_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_stage_info = {};
    frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage_info.module = frag_module;
    frag_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage_info, frag_stage_info};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex3);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex3, pos);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex3, color);
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex3, uv);

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &bindingDescription;
    vertex_input_info.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertex_input_info.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(context.GetContext().swapchain.extent.width);
    viewport.height = static_cast<float>(context.GetContext().swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = context.GetContext().swapchain.extent;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &colorBlendAttachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    auto descriptor_set_Layout = descriptorSetLayout->getDescriptorSetLayout();
    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_set_Layout;
    // pipeline_layout_info.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(context.GetContext().device.device, &pipeline_layout_info, nullptr, &pipeline_layout)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout\n");
    }

    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_info = {};
    dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_info.pDynamicStates = dynamic_states.data();

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_info;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(context.GetContext().device.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
                                  &graphics_pipeline) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create pipline\n");
    }

    vkDestroyShaderModule(context.GetContext().device.device, frag_module, nullptr);
    vkDestroyShaderModule(context.GetContext().device.device, vert_module, nullptr);
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

void DrawModel::create_render_pass() {
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = context.GetContext().swapchain.image_format;
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

    if (vkCreateRenderPass(context.GetContext().device.device, &render_pass_info, nullptr, &render_pass) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass");
    }
}
} // end namespace lvk
