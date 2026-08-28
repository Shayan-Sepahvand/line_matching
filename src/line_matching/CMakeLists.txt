# ============================================================================
# Line matching library
# ============================================================================
# OpenCV is already found in the root CMakeLists.txt.
# DO NOT call find_package(OpenCV ...) again here.


add_library(
    line_matching
    SHARED
    line_matching.cpp
)


target_include_directories(
    line_matching

    PUBLIC

        ${PROJECT_SOURCE_DIR}/include
        ${OpenCV_INCLUDE_DIRS}
)


target_link_libraries(
    line_matching

    PUBLIC

        klt
        EDLine
        ${OpenCV_LIBS}
)
