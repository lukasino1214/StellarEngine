set(MAIN_PROJECT ON)
add_subdirectory(json)
set(FX_GLTF_INSTALL OFF CACHE BOOL "" FORCE)
set(FX_GLTF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(fx-gltf)
#add_subdirectory(VulkanMemoryAllocator)
add_subdirectory(imgui)
add_subdirectory(ImGuizmo)
add_subdirectory(JoltPhysics/Build)

#if (DEFINED VULKAN_SDK_PATH)
#    set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include") # 1.1 Make sure this include path is correct
#    set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib") # 1.2 Make sure lib path is correct
#    set(Vulkan_FOUND "True")
#else()
#    find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
#    message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
#endif()
#if (NOT Vulkan_FOUND)
#    message(FATAL_ERROR "Could not find Vulkan library!")
#else()
#    message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
#endif()
#
#find_package(Vulkan REQUIRED)
#find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
#link_libraries(${Vulkan_LIBRARY})