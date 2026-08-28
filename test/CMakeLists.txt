# OpenCV is already found in the root CMakeLists.txt.
# Do NOT call find_package(OpenCV ...) again here.

# ============================================================================
# EDLine test
# ============================================================================

add_executable(
    test_edline_detector
    test_edline_detector.cpp
)

target_include_directories(
    test_edline_detector
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(
    test_edline_detector
    PRIVATE
        EDLine
        ${OpenCV_LIBS}
)


# ============================================================================
# Line matching test
# ============================================================================

add_executable(
    test_line_matching
    test_line_matching.cpp
)

target_include_directories(
    test_line_matching
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(
    test_line_matching
    PRIVATE
        line_matching
        klt
        EDLine
        ${OpenCV_LIBS}
)
