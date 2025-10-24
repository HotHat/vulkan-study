cmake_minimum_required(VERSION 3.29)
project(vulkan)

set(CMAKE_CXX_STANDARD 20)

add_executable(vulkan src/main.cpp
        src/HelloTriangle.h
        src/HelloTriangle.cpp
)


set(VK_INFO
        src/main3.cpp
        src/VkInfo.cpp
        src/LkInstance.cpp
        src/LkDevice.cpp
)
add_executable(triangle src/main2.cpp )


target_include_directories(vulkan PUBLIC C:/VulkanSDK/Include)

target_link_libraries(vulkan
        C:/VulkanSDK/Lib/vulkan-1.lib
        C:/VulkanSDK/Lib/glfw/glfw3.lib
        )

target_include_directories(triangle PUBLIC C:/VulkanSDK/Include)
target_link_libraries(triangle
        C:/VulkanSDK/Lib/vulkan-1.lib
        C:/VulkanSDK/Lib/glfw/glfw3.lib )

add_custom_command(TARGET vulkan POST_BUILD
        COMMAND  ${CMAKE_COMMAND} ARGS -E make_directory ${CMAKE_BINARY_DIR}/shaders)
add_custom_command(TARGET vulkan POST_BUILD
        COMMAND  ${CMAKE_COMMAND} ARGS -E make_directory ${CMAKE_BINARY_DIR}/textures)

add_custom_command(TARGET vulkan POST_BUILD
        COMMAND  ${CMAKE_COMMAND} ARGS -E copy
        ${CMAKE_SOURCE_DIR}/textures/texture.jpg
        ${CMAKE_BINARY_DIR}/textures/texture.jpg)

add_custom_command(TARGET vulkan POST_BUILD
        COMMAND C:/VulkanSDK/Bin/glslc ${CMAKE_SOURCE_DIR}/shaders/shader.vert -o ${CMAKE_BINARY_DIR}/shaders/vert.spv)

add_custom_command(TARGET vulkan POST_BUILD
        COMMAND  C:/VulkanSDK/Bin/glslc ${CMAKE_SOURCE_DIR}/shaders/shader.frag -o ${CMAKE_BINARY_DIR}/shaders/frag.spv)


# vulkan info
add_executable(vk_info ${VK_INFO})
target_include_directories(vk_info PUBLIC C:/VulkanSDK/Include)
target_link_libraries(vk_info
        C:/VulkanSDK/Lib/vulkan-1.lib
        C:/VulkanSDK/Lib/glfw/glfw3.lib
)
