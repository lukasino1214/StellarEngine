add_library(IMGUIZMO STATIC)


file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(IMGUIZMO
        PRIVATE
        ${SRC_FILES}
        )

target_include_directories( IMGUIZMO
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
        )

target_link_libraries(IMGUIZMO LINK_PUBLIC IMGUI)