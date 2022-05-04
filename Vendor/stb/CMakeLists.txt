add_library(STB STATIC)


file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
target_sources(STB
        PRIVATE
        ${SRC_FILES}
        )

target_include_directories( STB
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
        )