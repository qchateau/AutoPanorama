#ifndef INNERCUTFINDER_H
#define INNERCUTFINDER_H

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#if defined(ENABLE_LOG) && ENABLE_LOG
#define LOG(x) std::cout << x
#define LOGLN(x) std::cout << x << std::endl
#else
#define LOG(x)
#define LOGLN(x)
#endif

namespace autopanorama {

class InnerCutFinder {
public:
    InnerCutFinder(cv::InputArray mask);
    cv::Rect getROI();
    void process();

private:
    cv::Rect processFirst();
    bool processLevel(int level);

    std::vector<cv::Mat> pyramid_;
    std::vector<cv::Rect> pyramid_roi_;

    int min_search_res_;
    double step_down_scale_;
    double roi_min_area_;
    bool done_;
    bool failed_;
};

} // autopanorama

#endif // INNERCUTFINDER_H
