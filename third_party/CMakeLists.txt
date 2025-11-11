

cmake_minimum_required(VERSION 3.29)

project(third_party LANGUAGES C CXX)


# vma
#add_library(vma INTERFACE)
#set(VMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vma/include)
#target_sources(vma INTERFACE ${VMA_DIR}/vk_mem_alloc.h)
#target_include_directories(vma SYSTEM INTERFACE ${VMA_DIR})
#target_link_libraries(vma INTERFACE vulkan)