cmake_minimum_required(VERSION 3.21)
project(Editor)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(Editor ${SRC_FILES} ${HEADER_FILES})
target_link_libraries(Editor LINK_PUBLIC Engine)

target_link_libraries(Editor LINK_PUBLIC IMGUI)
target_link_libraries(Editor LINK_PUBLIC IMGUIZMO)