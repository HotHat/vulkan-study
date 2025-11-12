cmake_minimum_required(VERSION 3.29)

project(framework LANGUAGES C CXX)

set(COMMON_FILES
        # Header Files
        common/error.h
        common/strings.h
        common/helpers.h
        common/error.cpp
        common/logging.hpp
        common/strings.cpp
)
set(CORE_FILES
        # Header Files
        vk.h
        allocated.h
        buffer.h
        builder_base.h
        hpp_allocated.h
        vk_common.h
        vulkan_resource.h
        vulkan_type_mapping.h
        glsl_compiler.h
        shader_module.h
        spirv_reflection.h
        # Source Files
        allocated.cpp
        vk_common.cpp
        glsl_compiler.cpp
        shader_module.cpp
        spirv_reflection.cpp
)

set(FILESYSTEM_FILES
        filesystem/legacy.h
        filesystem/filesystem.hpp
        filesystem/filesystem.cpp
        filesystem/legacy.cpp
        filesystem/std_filesystem.hpp
        filesystem/std_filesystem.cpp
)



set(PROJECT_FILES
        ${COMMON_FILES}
        ${CORE_FILES}
        ${FILESYSTEM_FILES})

#NB: switch this to shared library and things stop working. (there is likely two copies of volk somewhere.
add_library(${PROJECT_NAME} OBJECT ${PROJECT_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)

# include
target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${VulkanSDKInclude}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
        stb
        spdlog
        glslang
        glslang-default-resource-limits
#        ${VulkanSDKLibDir}/glslang.lib
        SPIRV
        spirv-cross-glsl
        ${VulkanSDKLib}
)
#target_link_libraries(${PROJECT_NAME} PUBLIC
#        VulkanSDKInclude)
