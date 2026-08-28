#include <iostream>
#include <string>
#include <vector>

#include "line_matching/line_matching.h"


int main(int argc, char** argv)
{
    // =========================================================================
    // Command-line arguments
    // =========================================================================

    if (argc != 3)
    {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "  " << argv[0]
                  << " <reference_image> <current_image>"
                  << std::endl;

        return 1;
    }

    std::string reference_image_path = argv[1];
    std::string current_image_path   = argv[2];


    // =========================================================================
    // Parameters
    // =========================================================================

    // 0: no visualization
    // 1: debug visualization
    // 2: more debug visualization
    int debug_show = 0;

    // Illumination adaptation for KLT tracker
    bool illumination_adapt = false;

    // Remove outliers using topological filtering
    bool topological_filter = true;

    // Distance used by line filtering
    float filter_distance = 3.0f;


    // =========================================================================
    // Read images
    // =========================================================================

    cv::Mat img_ref = cv::imread(
        reference_image_path,
        cv::IMREAD_GRAYSCALE
    );

    cv::Mat img_cur = cv::imread(
        current_image_path,
        cv::IMREAD_GRAYSCALE
    );

    if (img_ref.empty())
    {
        std::cerr << "ERROR: Could not read reference image:" << std::endl;
        std::cerr << reference_image_path << std::endl;
        return 1;
    }

    if (img_cur.empty())
    {
        std::cerr << "ERROR: Could not read current image:" << std::endl;
        std::cerr << current_image_path << std::endl;
        return 1;
    }


    // =========================================================================
    // Camera intrinsic matrix
    // =========================================================================

    cv::Mat K = (cv::Mat_<float>(3, 3) <<
        436.23459, 0,         364.44122,
        0,         436.23459, 256.95169,
        0,         0,         1
    );


    // =========================================================================
    // EDLine detector
    // =========================================================================

    // ksize,
    // sigma,
    // gradientThreshold,
    // anchorThreshold,
    // scanIntervals,
    // minLineLen,
    // lineFitErrThreshold

    EDLineParam param = {
        5,
        1.0,
        30,
        5,
        2,
        25,
        1.8
    };

    EDLineDetector line_detector(param);


    // =========================================================================
    // Detect lines in reference image
    // =========================================================================

    std::vector<Line> lines_ref;

    line_detector.EDline(
        img_ref,
        lines_ref,
        false
    );


    // =========================================================================
    // Detect lines in current image
    // =========================================================================

    std::vector<Line> lines_cur;

    line_detector.EDline(
        img_cur,
        lines_cur,
        false
    );


    // =========================================================================
    // Line matching
    // =========================================================================

    LineMatching line_matching;

    line_matching.LineFilter(
        lines_ref,
        filter_distance
    );

    line_matching.LineFilter(
        lines_cur,
        filter_distance
    );


    std::vector<int> line_ref_to_line_cur;


    // No reference transformation is supplied.
    //
    // Use an empty cv::Mat rather than:
    //
    //     *(cv::Mat*)NULL
    //
    // which is unsafe.

    cv::Mat empty_T;


    line_matching.Matching(
        img_ref,
        img_cur,
        lines_ref,
        lines_cur,
        line_ref_to_line_cur,
        K,
        K,
        empty_T,
        illumination_adapt,
        topological_filter,
        debug_show,
        0,
        1
    );


    // =========================================================================
    // Count matches
    // =========================================================================

    int num_matches = 0;

    for (size_t i = 0; i < line_ref_to_line_cur.size(); ++i)
    {
        if (line_ref_to_line_cur[i] >= 0)
        {
            num_matches++;
        }
    }


    // =========================================================================
    // Print results
    // =========================================================================

    std::cout << std::endl;
    std::cout << "Reference image: "
              << reference_image_path
              << std::endl;

    std::cout << "Current image:   "
              << current_image_path
              << std::endl;

    std::cout << "Reference lines: "
              << lines_ref.size()
              << std::endl;

    std::cout << "Current lines:   "
              << lines_cur.size()
              << std::endl;

    std::cout << "Matched lines:   "
              << num_matches
              << std::endl;

    std::cout << std::endl;


    return 0;
}
