//
// Created by admin on 2025/11/14.
//

#include "draw_model.h"

#include <array>
#include <iostream>
#include <ostream>
#include <vector>
#include "allocator.h"
#include "functions.h"
#include <glm/gtc/matrix_transform.hpp>

namespace lvk {
DrawModel::DrawModel(VulkanContext &context) : context(context) {
    // create_render_pass();
    render_pass = context.GetDefaultRenderPass();
    allocator = std::make_unique<Allocator>(context);

    load3();
    createDescriptorSet();
    CreateGraphicsPipeline2();
}

void DrawModel::load2() {
    vertices = {
        {{-0.0f, -0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
    };

    indices = {0, 1, 2, 2, 3, 0};

    uint32_t vertice_size = sizeof(vertices[0]) * vertices.size();
    uint32_t indices_size = sizeof(indices[0]) * indices.size();

    vertex_buffer = allocator->CreateBuffer(vertice_size,
                                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                            VMA_MEMORY_USAGE_CPU_TO_GPU);
    indices_buffer = allocator->CreateBuffer(indices_size,
                                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                             VMA_MEMORY_USAGE_CPU_TO_GPU);


    vertex_buffer->CopyData(vertice_size, (void *) vertices.data());
    vertex_buffer->Flush(0, vertice_size);

    indices_buffer->CopyData(indices_size, (void *) indices.data());
    indices_buffer->Flush(0, indices_size);
}

void DrawModel::load3() {
    vertices = {
        // {{-0.0f, -0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        // {{0.5f, -0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        // {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        // {{-0.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
        {{100.0f, 100.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{200.0f, 100.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{200.0f, 200.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{100.0f, 200.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}
    };

    indices = {0, 1, 2, 2, 3, 0};

    uint32_t vertice_size = sizeof(vertices[0]) * vertices.size();
    uint32_t indices_size = sizeof(indices[0]) * indices.size();

    vertex_buffer = allocator->CreateBuffer(vertice_size,
                                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                            VMA_MEMORY_USAGE_CPU_TO_GPU);
    indices_buffer = allocator->CreateBuffer(indices_size,
                                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                             VMA_MEMORY_USAGE_CPU_TO_GPU);


    vertex_buffer->CopyData(vertice_size, (void *) vertices.data());
    vertex_buffer->Flush(0, vertice_size);

    indices_buffer->CopyData(indices_size, (void *) indices.data());
    indices_buffer->Flush(0, indices_size);

    ubo_buffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

    auto extent = context.swapchain.extent;
    auto view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    auto projection = glm::ortho(0.0f, static_cast<float>(extent.width), 0.0f, static_cast<float>(extent.height), -5.0f,
                                 5.0f);

    auto model = glm::mat4(1.0f);
    globalUbo.mvp = projection * view * model;
    // globalUbo.mvp[1][1] *= -1;

    // globalUbo.mvp = glm::mat4(1.0f);

    for (int i = 0; i < vertices.size(); i++) {
        auto tp = globalUbo.mvp * glm::vec4(vertices[i].pos, 1.0f);
        std::cout << tp[0] << " " << tp[1] << " " << tp[2] << " " << tp[3] << std::endl;
    }

    for (int i = 0; i < ubo_buffers.size(); i++) {
        ubo_buffers[i] = allocator->CreateBuffer2(sizeof(GlobalUbo),
                                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                  VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                  VMA_ALLOCATION_CREATE_MAPPED_BIT
        );

        // ubo_buffers[i]->CopyData(sizeof(GlobalUbo), (void *) &globalUbo);
        // ubo_buffers[i]->Flush();
    }
    UpdateUniform(globalUbo);
}

void DrawModel::load() {
    vertices = {
        {{-0.0f, -0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
    };

    indices = {0, 1, 2, 2, 3, 0};

    uint32_t vertice_size = sizeof(vertices[0]) * vertices.size();
    uint32_t indices_size = sizeof(indices[0]) * indices.size();

    vertex_buffer = allocator->CreateBuffer(vertice_size,
                                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                            VMA_MEMORY_USAGE_CPU_TO_GPU);
    indices_buffer = allocator->CreateBuffer(indices_size,
                                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                             VMA_MEMORY_USAGE_CPU_TO_GPU);


    vertex_buffer->CopyData(vertice_size, (void *) vertices.data());
    vertex_buffer->Flush(0, vertice_size);

    indices_buffer->CopyData(indices_size, (void *) indices.data());
    indices_buffer->Flush(0, indices_size);
}

void DrawModel::destroy() {
    vertex_buffer->Destroy();
    indices_buffer->Destroy();

    for (int i = 0; i < ubo_buffers.size(); i++) {
        ubo_buffers[i]->Destroy();
    }

    allocator->Destroy();

    descriptorPool->Cleanup();
    descriptorSetLayout->Cleanup();

    vkDestroyPipeline(context.device.device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(context.device.device, pipeline_layout, nullptr);
}

void DrawModel::draw(RenderContext &context) {
    // auto current_frame_ = context.image_index;
    auto current_image_index = context.GetCurrentImageIndex();
    auto commandBuffer = context.GetCurrentCommandBuffer();

    VkBuffer vertexBuffers[] = {vertex_buffer->buffer};
    VkDeviceSize offsets[] = {0};

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indices_buffer->buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                            &descriptorSets[current_image_index], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void DrawModel::CreateGraphicsPipeline() {
    auto vert_code = ReadFile("../shaders/vertbuffer.vert.spv");
    auto frag_code = ReadFile("../shaders/vertbuffer.frag.spv");

    VkShaderModule vert_module = CreateShaderModule(context.device.device, vert_code);
    VkShaderModule frag_module = CreateShaderModule(context.device.device, frag_code);
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
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

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
    viewport.width = static_cast<float>(context.swapchain.extent.width);
    viewport.height = static_cast<float>(context.swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = context.swapchain.extent;

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

    if (vkCreatePipelineLayout(context.device.device, &pipeline_layout_info, nullptr, &pipeline_layout)
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

    if (vkCreateGraphicsPipelines(context.device.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
                                  &graphics_pipeline) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create pipline\n");
    }

    vkDestroyShaderModule(context.device.device, frag_module, nullptr);
    vkDestroyShaderModule(context.device.device, vert_module, nullptr);
}

void DrawModel::CreateGraphicsPipeline2() {
    auto vert_code = ReadFile("../shaders/ubo.vert.spv");
    auto frag_code = ReadFile("../shaders/ubo.frag.spv");

    VkShaderModule vert_module = CreateShaderModule(context.device.device, vert_code);
    VkShaderModule frag_module = CreateShaderModule(context.device.device, frag_code);
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
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

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
    viewport.width = static_cast<float>(context.swapchain.extent.width);
    viewport.height = static_cast<float>(context.swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = context.swapchain.extent;

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

    if (vkCreatePipelineLayout(context.device.device, &pipeline_layout_info, nullptr, &pipeline_layout)
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

    if (vkCreateGraphicsPipelines(context.device.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
                                  &graphics_pipeline) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create pipline\n");
    }

    vkDestroyShaderModule(context.device.device, frag_module, nullptr);
    vkDestroyShaderModule(context.device.device, vert_module, nullptr);
}

void DrawModel::UpdateUniform(GlobalUbo &ubo) {
    for (int i = 0; i < ubo_buffers.size(); i++) {
        ubo_buffers[i]->CopyData(sizeof(GlobalUbo), (void *) &ubo);
        ubo_buffers[i]->Flush();
    }
}

void DrawModel::createDescriptorSet() {
    descriptorPool =
            DescriptorPool::Builder(context.device)
            .SetMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .Build();

    descriptorSetLayout =
            DescriptorSetLayout::Builder(context.device)
            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .Build();

    descriptorSets.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < descriptorSets.size(); i++) {
        // auto bufferInfo = ubo_buffers[i]->descriptorInfo();
        auto range = ubo_buffers[i]->size;

        auto bufferInfo = VkDescriptorBufferInfo{
            ubo_buffers[i]->buffer,
            0,
            // VK_WHOLE_SIZE
            // ubo_buffers[i]->size,
            range
        };

        DescriptorWriter(*descriptorSetLayout, *descriptorPool)
                .WriteBuffer(0, &bufferInfo)
                .Build(descriptorSets[i]);
    }
}

void DrawModel::create_render_pass() {
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = context.swapchain.image_format;
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

    if (vkCreateRenderPass(context.device.device, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass");
    }
}
} // end namespace lvk
