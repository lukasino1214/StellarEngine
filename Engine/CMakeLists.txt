cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

add_library(Engine STATIC system/deferred_rendering_system.cpp system/deferred_rendering_system.h)

set_project_warnings(Engine)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/)
target_sources(Engine PRIVATE ${SRC_FILES})

target_include_directories(Engine PUBLIC ${HEADER_FILES})

find_package(EnTT CONFIG REQUIRED)
target_link_libraries(Engine PRIVATE EnTT::EnTT)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(Engine PRIVATE glfw)

find_package(glm CONFIG REQUIRED)
target_link_libraries(Engine PRIVATE glm::glm)

find_package(unofficial-shaderc_util CONFIG REQUIRED)
target_link_libraries(Engine PRIVATE unofficial::shaderc_util::shaderc_util)

find_package(unofficial-shaderc CONFIG REQUIRED)
target_link_libraries(Engine PRIVATE unofficial::shaderc::shaderc)

find_package(yaml-cpp CONFIG REQUIRED)
target_link_libraries(Engine PRIVATE yaml-cpp)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(Engine PUBLIC spdlog::spdlog spdlog::spdlog_header_only)

find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")
target_include_directories(Engine PRIVATE ${STB_INCLUDE_DIRS})

target_link_libraries(Engine LINK_PUBLIC IMGUI)
target_link_libraries(Engine LINK_PUBLIC IMGUIZMO)
target_link_libraries(Engine LINK_PUBLIC fx-gltf)
#target_link_libraries(Engine LINK_PUBLIC Jolt)
target_link_libraries(Engine LINK_PUBLIC ${Vulkan_LIBRARY})