cmake_minimum_required(VERSION 3.29)
project(vulkan)

set(CMAKE_CXX_STANDARD 20)


set(VulkanSDK C:/VulkanSDK)
set(VulkanSDKInclude ${VulkanSDK}/Include)
set(VulkanSDKLibDir ${VulkanSDK}/Lib)
set(VulkanSDKLib ${VulkanSDK}/Lib/vulkan-1.lib)

# Add third party libraries
add_subdirectory(third_party)

# Add core
#add_subdirectory(framework)

# Add lvk
add_subdirectory(lvk)


add_executable(vulkan src/main.cpp
        src/HelloTriangle.h
        src/HelloTriangle.cpp)



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
set(VK_INFO
        src/main3.cpp
        src/VkInfo.cpp
        src/LkInstance.cpp
        src/LkDevice.cpp)

add_executable(vk_info ${VK_INFO})
target_include_directories(vk_info PUBLIC VulkanSDKInclude)
target_link_libraries(vk_info
        VulkanSDKLib
        C:/VulkanSDK/Lib/glfw/glfw3.lib)


set(VK_INFO2
        src/main4.cpp)

add_executable(vk_info2 ${VK_INFO2})
target_include_directories(vk_info2 PUBLIC lvk)
target_link_libraries(vk_info2
        C:/VulkanSDK/Lib/glfw/glfw3.lib
        lvk
)


set(VK_INFO3
        src/main5.cpp)

add_executable(vk_info3 ${VK_INFO3})
target_include_directories(vk_info3 PUBLIC lvk)
target_link_libraries(vk_info3
        C:/VulkanSDK/Lib/glfw/glfw3.lib
        lvk
)