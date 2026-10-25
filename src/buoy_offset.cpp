#include <iostream>
#include <vector>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <cmath>
#include <optional>
#include <cstdint>

struct Detection{
    int id;
    int class_id;
    double cx;
    double cy;
    double width;
    double height;
    double confidence;
    std::optional<std::int64_t> stamp_ns;
};
Detection detection;

enum class Side{
    Right,
    Left,
    Center
};


struct BuoyOffset{
    bool valid;
    double dx_px;
    double dx_norm;
    Side side;
    std::optional<std::int64_t> stamp_ns;
    double confidence;
};

BuoyOffset compute_buoy_offset(
    const Detection& detection,
    int image_width,
    double conf_threshold = 0.35,
    double center_tolerance_norm = 0.08){

    BuoyOffset result;
    result.valid=false;

    if(image_width<=0){
        return result;
}

    if(conf_threshold>detection.confidence){
        return result;
}

    const double dx_px = detection.cx - image_width / 2.0;
    const double dx_norm = dx_px / (image_width / 2.0);

    if (std::fabs(dx_norm) <= center_tolerance_norm) {
        result.side=Side::Center;
}
    else if (dx_norm < 0) {
        result.side=Side::Left;
}
    else {
        result.side=Side::Right;
}

    result.dx_px=dx_px;
    result.dx_norm=dx_norm;
    result.confidence=detection.confidence;
    result.stamp_ns=detection.stamp_ns;
    result.valid=true;

    return result;
}





// 最小检测流程：读取图像、提取红色区域并保留面积最大的目标。
// 固定 HSV 阈值仅面向仓库样本，不适用于真实水面环境。
bool DetectRedBuoy(const cv::Mat& image, cv::Rect& buoy_box) {
    cv::Mat hsv;
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);

    cv::Mat red_low;
    cv::Mat red_high;
    cv::Mat red_mask;
    cv::inRange(hsv, cv::Scalar(0, 90, 70), cv::Scalar(10, 255, 255), red_low);
    cv::inRange(hsv, cv::Scalar(170, 90, 70), cv::Scalar(179, 255, 255), red_high);
    cv::bitwise_or(red_low, red_high, red_mask);

    const cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(red_mask, red_mask, cv::MORPH_OPEN, kernel);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(red_mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    double largest_area = 200.0;
    bool found = false;
    for (const auto& contour : contours) {
        const double area = cv::contourArea(contour);
        if (area > largest_area) {
            largest_area = area;
            buoy_box = cv::boundingRect(contour);
            found = true;
        }
    }
    return found;
}

// TODO: 计算浮漂外接框的水平中心坐标。
int BuoyCenterX(int buoy_x, int buoy_width){
   int coordinate;
    coordinate=buoy_x+buoy_width/2;
    return coordinate;
}


int HorizontalOffsetPx(int buoy_center_x, int image_width) {
    int image_coordinate;
    image_coordinate=image_width/2;
    int offset;
    offset=buoy_center_x-image_coordinate;
    return offset;
}

// TODO: 返回 LEFT、CENTER 或 RIGHT，绝对偏移不超过 tolerance_px 时返回 CENTER。
const char* RelativePosition(int offset_px, int tolerance_px) {
    if (offset_px>tolerance_px){
        return "RIGHT";
    }
    else if (offset_px<-tolerance_px){
        return "LEFT";
    }

    else{
        return "CENTER";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: buoy_offset <image.jpg>\n";
        return 1;
    }

    const cv::Mat image = cv::imread(argv[1], cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Error: cannot read image: " << argv[1] << '\n';
        return 1;
    }

    cv::Rect buoy_box;
    if (!DetectRedBuoy(image, buoy_box)) {
        std::cout << "status=NOT_FOUND\n";
        return 0;
    }

    const int buoy_center_x = BuoyCenterX(buoy_box.x, buoy_box.width);
    const int offset_px = HorizontalOffsetPx(buoy_center_x, image.cols);
    const char* position = RelativePosition(offset_px, 40);

    std::cout << "status=FOUND\n"
              << "image_width=" << image.cols << '\n'
              << "buoy_x=" << buoy_box.x << '\n'
              << "buoy_width=" << buoy_box.width << '\n'
              << "buoy_center_x=" << buoy_center_x << '\n'
              << "offset_px=" << offset_px << '\n'
              << "position=" << position << '\n';
    return 0;
}
