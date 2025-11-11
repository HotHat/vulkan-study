cmake_minimum_required(VERSION 3.29)

project(framework LANGUAGES C CXX)

set(CORE_FILES
# Header Files
        allocated.h
        buffer.h
        builder_base.h
        hpp_allocated.h
        vk_common.h
        vulkan_resource.h
        vulkan_type_mapping.h
# Source Files
        allocated.cpp
        vk_common.cpp
)

source_group("core\\" FILES ${CORE_FILES})

set(PROJECT_FILES
        ${CORE_FILES})

#NB: switch this to shared library and things stop working. (there is likely two copies of volk somewhere.
add_library(${PROJECT_NAME} OBJECT ${PROJECT_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(${PROJECT_NAME} PUBLIC ${VulkanSDKInclude})

#target_link_libraries(${PROJECT_NAME} PUBLIC
#        VulkanSDKInclude)
