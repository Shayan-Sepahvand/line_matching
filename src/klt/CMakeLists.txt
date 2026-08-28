set(OpenCV_DIR
    "/home/shayan/ldetection/opencv34/opencv-3.4.20/build"
)

find_package(
    OpenCV 3.4 REQUIRED
    PATHS
        "/home/shayan/ldetection/opencv34/opencv-3.4.20/build"
    NO_DEFAULT_PATH
)

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")

set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -O3 -msse4.2 -mavx"
)

set(
    KLT_SOURCES
    klt.cpp
    lk_tracker_invoker_ori.cpp
    lk_tracker_invoker_1d.cpp
    lk_tracker_invoker_2d.cpp
    lk_tracker_invoker_2d_single.cpp
)

add_library(
    klt
    SHARED
    ${KLT_SOURCES}
)

target_include_directories(
    klt

    PUBLIC

        ${OpenCV_INCLUDE_DIRS}

        ${PROJECT_SOURCE_DIR}/include

        # Needed because klt.h includes lkpyramid.hpp
        /home/shayan/ldetection/opencv34/opencv-3.4.20/modules/video/src

        # Needed because klt.h includes opencv2/ts.hpp
        /home/shayan/ldetection/opencv34/opencv-3.4.20/modules/ts/include
)

target_link_libraries(
    klt

    PUBLIC

        ${OpenCV_LIBS}
)
