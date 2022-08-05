cmake_minimum_required(VERSION 3.10)

set(WARNINGS_AS_ERRORS TRUE)
include(cmake/warnings.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_TOOLCHAIN_FILE /home/lukas/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(All)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
)

add_subdirectory(Vendor)


add_subdirectory(Engine)
add_subdirectory(Editor)