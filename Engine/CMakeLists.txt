cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

add_library(Engine STATIC system/deferred_rendering_system.cpp system/deferred_rendering_system.h)

set_project_warnings(Engine)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/)
target_sources(Engine PRIVATE ${SRC_FILES})

target_include_directories(Engine PUBLIC ${HEADER_FILES})

target_link_libraries(Engine LINK_PUBLIC IMGUI)
target_link_libraries(Engine LINK_PUBLIC IMGUIZMO)
target_link_libraries(Engine LINK_PUBLIC STB)
target_link_libraries(Engine LINK_PUBLIC EnTT)
target_link_libraries(Engine LINK_PUBLIC spdlog)
target_link_libraries(Engine LINK_PUBLIC fx-gltf)
#target_link_libraries(Engine LINK_PUBLIC Jolt)
target_link_libraries(Engine LINK_PUBLIC ${Vulkan_LIBRARY})
target_link_libraries(Engine LINK_PUBLIC glfw)
target_link_libraries(Engine LINK_PUBLIC shaderc)
target_link_libraries(Engine LINK_PUBLIC yaml-cpp)
target_link_libraries(Engine LINK_PUBLIC glm)