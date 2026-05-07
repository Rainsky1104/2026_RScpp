#include <iostream>
#include "SatelliteImage.h"
#include <opencv2/opencv.hpp>
int main() {
    cv::setNumThreads(std::thread::hardware_concurrency()); // 设置OpenCV多线程，线程数为CPU核心数，最大化性能
}
    