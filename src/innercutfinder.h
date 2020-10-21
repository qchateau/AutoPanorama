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

    std::vector<cv::Mat> pyramid;
    std::vector<cv::Rect> pyramid_roi;

    int min_search_res;
    double step_down_scale;
    double roi_min_area;
    bool done, failed;
};

} // autopanorama

#endif // INNERCUTFINDER_H
