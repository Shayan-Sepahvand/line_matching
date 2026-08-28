#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>

#include "line_matching/line_matching.h"


// ============================================================================
// Create output directory
// ============================================================================

bool createDirectory(const std::string& path)
{
    if (mkdir(path.c_str(), 0755) == 0)
        return true;

    if (errno == EEXIST)
        return true;

    return false;
}


// ============================================================================
// Extract start/end points from this repository's Line structure
//
// struct Line {
//     std::array<float, 4> line_endpoint;
//     ...
// };
//
// line_endpoint = [x1, y1, x2, y2]
// ============================================================================

cv::Point2f getStartPoint(const Line& line)
{
    return cv::Point2f(
        line.line_endpoint[0],
        line.line_endpoint[1]
    );
}


cv::Point2f getEndPoint(const Line& line)
{
    return cv::Point2f(
        line.line_endpoint[2],
        line.line_endpoint[3]
    );
}


// ============================================================================
// Line midpoint
// ============================================================================

cv::Point getLineMidpoint(
    const Line& line,
    int x_offset = 0,
    int y_offset = 0)
{
    cv::Point2f p1 = getStartPoint(line);
    cv::Point2f p2 = getEndPoint(line);

    return cv::Point(
        cvRound(0.5f * (p1.x + p2.x)) + x_offset,
        cvRound(0.5f * (p1.y + p2.y)) + y_offset
    );
}


// ============================================================================
// Draw line segment
// ============================================================================

void drawSegment(
    cv::Mat& image,
    const Line& line,
    const cv::Scalar& color,
    int thickness = 1,
    int x_offset = 0,
    int y_offset = 0)
{
    cv::Point2f p1 = getStartPoint(line);
    cv::Point2f p2 = getEndPoint(line);

    cv::Point start(
        cvRound(p1.x) + x_offset,
        cvRound(p1.y) + y_offset
    );

    cv::Point end(
        cvRound(p2.x) + x_offset,
        cvRound(p2.y) + y_offset
    );

    cv::line(
        image,
        start,
        end,
        color,
        thickness,
        cv::LINE_AA
    );
}


// ============================================================================
// Save detected lines
// ============================================================================

void saveDetectedLines(
    const cv::Mat& gray_image,
    const std::vector<Line>& lines,
    const std::string& output_path)
{
    cv::Mat output;

    cv::cvtColor(
        gray_image,
        output,
        cv::COLOR_GRAY2BGR
    );

    for (size_t i = 0; i < lines.size(); ++i)
    {
        drawSegment(
            output,
            lines[i],
            cv::Scalar(0, 255, 0),
            2
        );
    }

    if (!cv::imwrite(output_path, output))
    {
        std::cerr
            << "ERROR: Could not save "
            << output_path
            << std::endl;
    }
}


// ============================================================================
// Save matching visualization
// ============================================================================

void saveMatchingImage(
    const cv::Mat& img_ref,
    const cv::Mat& img_cur,
    const std::vector<Line>& lines_ref,
    const std::vector<Line>& lines_cur,
    const std::vector<int>& line_ref_to_line_cur,
    const std::string& output_path,
    double extraction_ms,
    double matching_ms)
{
    // ------------------------------------------------------------------------
    // Convert to color
    // ------------------------------------------------------------------------

    cv::Mat ref_color;
    cv::Mat cur_color;

    cv::cvtColor(
        img_ref,
        ref_color,
        cv::COLOR_GRAY2BGR
    );

    cv::cvtColor(
        img_cur,
        cur_color,
        cv::COLOR_GRAY2BGR
    );


    // ------------------------------------------------------------------------
    // Create side-by-side canvas
    // ------------------------------------------------------------------------

    const int header_height = 50;

    const int canvas_width =
        ref_color.cols +
        cur_color.cols;

    const int canvas_height =
        std::max(
            ref_color.rows,
            cur_color.rows
        ) +
        header_height;


    cv::Mat canvas(
        canvas_height,
        canvas_width,
        CV_8UC3,
        cv::Scalar(0, 0, 0)
    );


    // Reference image
    ref_color.copyTo(
        canvas(
            cv::Rect(
                0,
                header_height,
                ref_color.cols,
                ref_color.rows
            )
        )
    );


    // Current image
    cur_color.copyTo(
        canvas(
            cv::Rect(
                ref_color.cols,
                header_height,
                cur_color.cols,
                cur_color.rows
            )
        )
    );


    // ------------------------------------------------------------------------
    // Draw all detected lines in gray
    // ------------------------------------------------------------------------

    for (size_t i = 0; i < lines_ref.size(); ++i)
    {
        drawSegment(
            canvas,
            lines_ref[i],
            cv::Scalar(100, 100, 100),
            1,
            0,
            header_height
        );
    }


    for (size_t i = 0; i < lines_cur.size(); ++i)
    {
        drawSegment(
            canvas,
            lines_cur[i],
            cv::Scalar(100, 100, 100),
            1,
            ref_color.cols,
            header_height
        );
    }


    // ------------------------------------------------------------------------
    // Draw matched line pairs
    // ------------------------------------------------------------------------

    cv::RNG rng(12345);

    int num_matches = 0;


    for (size_t i = 0;
         i < line_ref_to_line_cur.size() &&
         i < lines_ref.size();
         ++i)
    {
        const int j =
            line_ref_to_line_cur[i];


        // Invalid/no match
        if (j < 0 ||
            j >= static_cast<int>(lines_cur.size()))
        {
            continue;
        }


        // Random but deterministic color for each match
        cv::Scalar color(
            rng.uniform(50, 255),
            rng.uniform(50, 255),
            rng.uniform(50, 255)
        );


        // ------------------------------------------------------------
        // Reference line
        // ------------------------------------------------------------

        drawSegment(
            canvas,
            lines_ref[i],
            color,
            2,
            0,
            header_height
        );


        // ------------------------------------------------------------
        // Current line
        // ------------------------------------------------------------

        drawSegment(
            canvas,
            lines_cur[j],
            color,
            2,
            ref_color.cols,
            header_height
        );


        // ------------------------------------------------------------
        // Connect the line centers
        // ------------------------------------------------------------

        cv::Point center_ref =
            getLineMidpoint(
                lines_ref[i],
                0,
                header_height
            );


        cv::Point center_cur =
            getLineMidpoint(
                lines_cur[j],
                ref_color.cols,
                header_height
            );


        cv::line(
            canvas,
            center_ref,
            center_cur,
            color,
            1,
            cv::LINE_AA
        );


        num_matches++;
    }


    // ------------------------------------------------------------------------
    // Vertical separator
    // ------------------------------------------------------------------------

    cv::line(
        canvas,
        cv::Point(
            ref_color.cols,
            header_height
        ),
        cv::Point(
            ref_color.cols,
            canvas.rows
        ),
        cv::Scalar(255, 255, 255),
        1
    );


    // ------------------------------------------------------------------------
    // Header text
    // ------------------------------------------------------------------------

    std::ostringstream text;

    text
        << "Matches: "
        << num_matches
        << " | Extraction: "
        << std::fixed
        << std::setprecision(2)
        << extraction_ms
        << " ms/image"
        << " | Matching: "
        << matching_ms
        << " ms";


    cv::putText(
        canvas,
        text.str(),
        cv::Point(10, 30),
        cv::FONT_HERSHEY_SIMPLEX,
        0.55,
        cv::Scalar(255, 255, 255),
        1,
        cv::LINE_AA
    );


    // ------------------------------------------------------------------------
    // Save
    // ------------------------------------------------------------------------

    if (!cv::imwrite(
            output_path,
            canvas))
    {
        std::cerr
            << "ERROR: Could not save matching image: "
            << output_path
            << std::endl;
    }
}


// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv)
{
    // =========================================================================
    // Arguments
    //
    // ./test_line_matching reference.png current.png [output_directory]
    // =========================================================================

    if (argc != 3 &&
        argc != 4)
    {
        std::cerr
            << "Usage:\n\n"
            << argv[0]
            << " <reference_image>"
            << " <current_image>"
            << " [output_directory]\n\n";

        return 1;
    }


    const std::string reference_image_path =
        argv[1];

    const std::string current_image_path =
        argv[2];


    std::string output_directory =
        "./results";


    if (argc == 4)
    {
        output_directory =
            argv[3];
    }


    // =========================================================================
    // Output directory
    // =========================================================================

    if (!createDirectory(output_directory))
    {
        std::cerr
            << "ERROR: Could not create output directory:\n"
            << output_directory
            << std::endl;

        return 1;
    }


    // =========================================================================
    // Parameters
    // =========================================================================

    // Disable imshow because OpenCV was built without GTK.
    int debug_show = 0;

    bool illumination_adapt =
        false;

    bool topological_filter =
        false;

    float filter_distance =
        3.0f;


    // =========================================================================
    // Read images
    // =========================================================================

    cv::Mat img_ref =
        cv::imread(
            reference_image_path,
            cv::IMREAD_GRAYSCALE
        );


    cv::Mat img_cur =
        cv::imread(
            current_image_path,
            cv::IMREAD_GRAYSCALE
        );


    if (img_ref.empty())
    {
        std::cerr
            << "ERROR: Could not read reference image:\n"
            << reference_image_path
            << std::endl;

        return 1;
    }


    if (img_cur.empty())
    {
        std::cerr
            << "ERROR: Could not read current image:\n"
            << current_image_path
            << std::endl;

        return 1;
    }


    // =========================================================================
    // Camera intrinsic matrix
    // =========================================================================

    cv::Mat K =
        (cv::Mat_<float>(3, 3) <<

            643.56796071, 0,         326.71147937,
            0,         640.86825351, 235.64925133,
            0,         0,         1
        );


    // =========================================================================
    // EDLine parameters
    // ============================================================================
// EDLineParam param =
// {
//     5,      // ksize
//     1.0,    // sigma
//     20,     // gradientThreshold   was 30
//     3,      // anchorThreshold     was 5
//     1,      // scanIntervals       was 2
//     15,     // minLineLen          was 25
//     2.5     // lineFitErrThreshold was 1.8
// };
    EDLineParam param =
        {
            5,
            1.0,
            15,
            2,
            1,
            10,
            3.0
        };


    EDLineDetector line_detector(param);

    LineMatching line_matching;


    std::vector<Line> lines_ref;

    std::vector<Line> lines_cur;


    typedef std::chrono::steady_clock Clock;


    // =========================================================================
    // REFERENCE IMAGE
    // =========================================================================


    // ------------------------------------------------------------------------
    // EDLine detection latency
    // ------------------------------------------------------------------------

    auto ref_detection_start =
        Clock::now();


    line_detector.EDline(
        img_ref,
        lines_ref,
        false
    );


    auto ref_detection_end =
        Clock::now();


    double ref_detection_ms =
        std::chrono::duration<double, std::milli>(
            ref_detection_end -
            ref_detection_start
        ).count();


    // ------------------------------------------------------------------------
    // Line filtering latency
    // ------------------------------------------------------------------------

    auto ref_filter_start =
        Clock::now();


    line_matching.LineFilter(
        lines_ref,
        filter_distance
    );


    auto ref_filter_end =
        Clock::now();


    double ref_filter_ms =
        std::chrono::duration<double, std::milli>(
            ref_filter_end -
            ref_filter_start
        ).count();


    // ------------------------------------------------------------------------
    // Total feature extraction latency
    // ------------------------------------------------------------------------

    double ref_extraction_ms =
        ref_detection_ms +
        ref_filter_ms;


    // =========================================================================
    // CURRENT IMAGE
    // =========================================================================


    // ------------------------------------------------------------------------
    // EDLine detection latency
    // ------------------------------------------------------------------------

    auto cur_detection_start =
        Clock::now();


    line_detector.EDline(
        img_cur,
        lines_cur,
        false
    );


    auto cur_detection_end =
        Clock::now();


    double cur_detection_ms =
        std::chrono::duration<double, std::milli>(
            cur_detection_end -
            cur_detection_start
        ).count();


    // ------------------------------------------------------------------------
    // Line filtering latency
    // ------------------------------------------------------------------------

    auto cur_filter_start =
        Clock::now();


    line_matching.LineFilter(
        lines_cur,
        filter_distance
    );


    auto cur_filter_end =
        Clock::now();


    double cur_filter_ms =
        std::chrono::duration<double, std::milli>(
            cur_filter_end -
            cur_filter_start
        ).count();


    // ------------------------------------------------------------------------
    // Total feature extraction latency
    // ------------------------------------------------------------------------

    double cur_extraction_ms =
        cur_detection_ms +
        cur_filter_ms;


    // =========================================================================
    // Feature extraction statistics
    // =========================================================================

    double average_detection_ms =
        (
            ref_detection_ms +
            cur_detection_ms
        ) / 2.0;


    double average_filter_ms =
        (
            ref_filter_ms +
            cur_filter_ms
        ) / 2.0;


    double average_extraction_ms =
        (
            ref_extraction_ms +
            cur_extraction_ms
        ) / 2.0;


    // =========================================================================
    // Line matching
    // ============================================================================

    std::vector<int>
        line_ref_to_line_cur;


    // No pose/reference transform
    cv::Mat empty_T;


    auto matching_start =
        Clock::now();


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


    auto matching_end =
        Clock::now();


    double matching_ms =
        std::chrono::duration<double, std::milli>(
            matching_end -
            matching_start
        ).count();


    // =========================================================================
    // Count matches
    // ============================================================================

    int num_matches = 0;


    for (size_t i = 0;
         i < line_ref_to_line_cur.size();
         ++i)
    {
        const int j =
            line_ref_to_line_cur[i];


        if (j >= 0 &&
            j < static_cast<int>(
                lines_cur.size()))
        {
            num_matches++;
        }
    }


    // =========================================================================
    // Save detected lines
    // ============================================================================

    const std::string ref_lines_path =
        output_directory +
        "/reference_lines.png";


    const std::string cur_lines_path =
        output_directory +
        "/current_lines.png";


    saveDetectedLines(
        img_ref,
        lines_ref,
        ref_lines_path
    );


    saveDetectedLines(
        img_cur,
        lines_cur,
        cur_lines_path
    );


    // =========================================================================
    // Save matching image
    // ============================================================================

    const std::string matches_path =
        output_directory +
        "/line_matches.png";


    saveMatchingImage(
        img_ref,
        img_cur,

        lines_ref,
        lines_cur,

        line_ref_to_line_cur,

        matches_path,

        average_extraction_ms,
        matching_ms
    );


    // =========================================================================
    // Save CSV
    // ============================================================================

    const std::string csv_path =
        output_directory +
        "/latency.csv";


    std::ofstream csv(
        csv_path.c_str()
    );


    if (!csv.is_open())
    {
        std::cerr
            << "ERROR: Could not create "
            << csv_path
            << std::endl;

        return 1;
    }


    csv
        << std::fixed
        << std::setprecision(6);


    csv
        << "metric,value\n";


    // Detection
    csv
        << "reference_detection_ms,"
        << ref_detection_ms
        << "\n";

    csv
        << "current_detection_ms,"
        << cur_detection_ms
        << "\n";

    csv
        << "average_detection_ms,"
        << average_detection_ms
        << "\n";


    // Filtering
    csv
        << "reference_filter_ms,"
        << ref_filter_ms
        << "\n";

    csv
        << "current_filter_ms,"
        << cur_filter_ms
        << "\n";

    csv
        << "average_filter_ms,"
        << average_filter_ms
        << "\n";


    // Total feature extraction
    csv
        << "reference_feature_extraction_ms,"
        << ref_extraction_ms
        << "\n";

    csv
        << "current_feature_extraction_ms,"
        << cur_extraction_ms
        << "\n";

    csv
        << "average_feature_extraction_ms,"
        << average_extraction_ms
        << "\n";


    // Matching
    csv
        << "matching_ms,"
        << matching_ms
        << "\n";


    // Counts
    csv
        << "reference_lines,"
        << lines_ref.size()
        << "\n";

    csv
        << "current_lines,"
        << lines_cur.size()
        << "\n";

    csv
        << "matches,"
        << num_matches
        << "\n";


    csv.close();


    // =========================================================================
    // Terminal output
    // ============================================================================

    std::cout
        << std::fixed
        << std::setprecision(3);


    std::cout
        << "\n"
        << "====================================================\n"
        << "KLT Line Matching Results\n"
        << "====================================================\n\n";


    std::cout
        << "Reference image:\n"
        << "  "
        << reference_image_path
        << "\n\n";


    std::cout
        << "Current image:\n"
        << "  "
        << current_image_path
        << "\n\n";


    // ------------------------------------------------------------------------
    // Features
    // ------------------------------------------------------------------------

    std::cout
        << "Reference lines: "
        << lines_ref.size()
        << "\n";


    std::cout
        << "Current lines:   "
        << lines_cur.size()
        << "\n";


    std::cout
        << "Matched lines:   "
        << num_matches
        << "\n\n";


    // ------------------------------------------------------------------------
    // Detection latency
    // ------------------------------------------------------------------------

    std::cout
        << "EDLine detection latency\n"
        << "------------------------------------\n";


    std::cout
        << "Reference: "
        << ref_detection_ms
        << " ms\n";


    std::cout
        << "Current:   "
        << cur_detection_ms
        << " ms\n";


    std::cout
        << "Average:   "
        << average_detection_ms
        << " ms/image\n\n";


    // ------------------------------------------------------------------------
    // Filtering latency
    // ------------------------------------------------------------------------

    std::cout
        << "Line filtering latency\n"
        << "------------------------------------\n";


    std::cout
        << "Reference: "
        << ref_filter_ms
        << " ms\n";


    std::cout
        << "Current:   "
        << cur_filter_ms
        << " ms\n";


    std::cout
        << "Average:   "
        << average_filter_ms
        << " ms/image\n\n";


    // ------------------------------------------------------------------------
    // Feature extraction latency
    // ------------------------------------------------------------------------

    std::cout
        << "Feature extraction latency\n"
        << "(EDLine + LineFilter)\n"
        << "------------------------------------\n";


    std::cout
        << "Reference: "
        << ref_extraction_ms
        << " ms\n";


    std::cout
        << "Current:   "
        << cur_extraction_ms
        << " ms\n";


    std::cout
        << "Average:   "
        << average_extraction_ms
        << " ms/image\n\n";


    // ------------------------------------------------------------------------
    // Matching
    // ------------------------------------------------------------------------

    std::cout
        << "Matching latency\n"
        << "------------------------------------\n";


    std::cout
        << matching_ms
        << " ms/image-pair\n\n";


    // ------------------------------------------------------------------------
    // Output files
    // ------------------------------------------------------------------------

    std::cout
        << "Saved files\n"
        << "------------------------------------\n";


    std::cout
        << ref_lines_path
        << "\n";


    std::cout
        << cur_lines_path
        << "\n";


    std::cout
        << matches_path
        << "\n";


    std::cout
        << csv_path
        << "\n";


    std::cout
        << "\n====================================================\n";


    return 0;
}
