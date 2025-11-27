cmake_minimum_required(VERSION 3.29)

project(lvk LANGUAGES C CXX)


set(LVK_FILES
        # Header Files
        device.h
        feature_chain.h
        functions.h
        instance.h
        render_context.h
        simple_draw.h
        system_info.h
        vulkan_context.h
        swapchain.h

        Vertex.h
        # Source Files
        device.cpp
        feature_chain.cpp
        functions.cpp
        instance.cpp
        render_context.cpp
        simple_draw.cpp
        system_info.cpp
        vulkan_context.cpp
        swapchain.cpp
        #
        allocator.cpp
        allocator.h
        buffer.h
        #
        draw_model.cpp
        descriptor.cpp
        descriptor.h
        draw_object.h
)

set(PROJECT_FILES
        ${LVK_FILES}
        Texture.h
        pipeline.cpp
        pipeline.h
)

#NB: switch this to shared library and things stop working. (there is likely two copies of volk somewhere.
add_library(${PROJECT_NAME} OBJECT ${PROJECT_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

# include
target_include_directories(${PROJECT_NAME} PUBLIC
        ${VulkanSDKInclude}
        ${CMAKE_CURRENT_SOURCE_DIR} )

target_link_libraries(${PROJECT_NAME} PUBLIC
        ${VulkanSDKLib}
        stb)

