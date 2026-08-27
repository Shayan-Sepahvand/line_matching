# Line Matching

This repository provides a robust C++ implementation for matching line features between two images. The algorithm is designed for high accuracy and resilience to viewpoint changes, illumination variations, and minor occlusions, making it suitable for applications like Visual Odometry (VO), SLAM, and image stitching.

### Core Techniques

The matching pipeline leverages a combination of classic and modern computer vision techniques:

1.  **Line Segment Detection**: Utilizes the **EDLines** algorithm for fast and accurate line segment extraction from images. A custom filtering step is applied to remove redundant parallel lines.

2.  **KLT-Based Point Tracking**: Instead of using complex line descriptors, this method samples a series of "anchor points" along each line in the reference image. It then tracks these points into the current image using a **1D-constrained Kanade-Lucas-Tomasi (KLT) optical flow**. This approach tracks points along their line's normal direction, which is more efficient and constrained. If camera pose and intrinsics are available, **epipolar geometry** is used to provide a precise initial guess for the KLT tracker.

3.  **Voting-Based Line Association**: A voting scheme aggregates the matches of individual anchor points to determine the corresponding line match. A line pair is considered a candidate match if it receives the majority of votes from the anchor points and satisfies a length consistency check.

4.  **Topological Filtering**: To ensure high-quality matches, a crucial **topological consistency check** is performed. This filter validates that the relative spatial relationships (e.g., sideness, distance) between pairs of matched lines are preserved across both images. Any match that violates this structural consistency with a significant number of other matches is rejected as an outlier.

This multi-stage approach, combining point-based tracking with strong geometric and topological constraints, results in a highly reliable line matching system.

Related project: [EDLine Parallel](https://github.com/HanjieLuo/EDLine_parallel)

![test image](./data/line_matching_result.png)

## References ## 
Papers Describing the Approach:

罗汉杰. 直线段匹配方法、装置、存储介质及终端[P]. 中国专利: CN109919190A, 2019-06-21.
http://luohanjie.com/2021-02-04/a-klt-based-line-segment-matching-algorithm.html

## Requirements ##
The code is tested on Ubuntu 14.04. It requires the following tools and libraries: CMake, OpenCV 3.4. 

## Building ##

```
#!bash
git clone https://github.com/HanjieLuo/line_matching.git
cd line_matching
mkdir build
cd build
cmake  ..
make
```

Test:

```
#!bash
./bin/test_line_matching
```

Test with EuRoc MAV dataset(MH_04_difficult):  
[![KLT-based Line Segment Matching Algorithm(Debug)](https://res.cloudinary.com/marcomontalbano/image/upload/v1612585399/video_to_markdown/images/youtube--3i1zt2bkSZc-c05b58ac6eb4c4700831b2b3070cd403.jpg)](https://youtu.be/3i1zt2bkSZc "KLT-based Line Segment Matching Algorithm(Debug)")

[![KLT-based Line Segment Matching Algorithm](https://res.cloudinary.com/marcomontalbano/image/upload/v1612585481/video_to_markdown/images/youtube--OQyB3OdJg4w-c05b58ac6eb4c4700831b2b3070cd403.jpg)](https://youtu.be/OQyB3OdJg4w "KLT-based Line Segment Matching Algorithm")

## Contact information ##
Hanjie Luo [luohanjie@gmail.com](mailto:luohanjie@gmail.com)


## This only works with opencv3.4 ##

```bash
cd /home/shayan

mkdir -p software
cd software

git clone --branch 3.4.20 --depth 1 \
    https://github.com/opencv/opencv.git \
    opencv-3.4.20


cmake \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_INSTALL_PREFIX=home/ldetection/opencv34/opencv-3.4-install \
    -D BUILD_TESTS=OFF \
    -D BUILD_PERF_TESTS=OFF \
    -D BUILD_EXAMPLES=OFF \
    -D BUILD_opencv_python2=OFF \
    -D BUILD_opencv_python3=OFF \
    -D BUILD_JAVA=OFF \
    ..

```


