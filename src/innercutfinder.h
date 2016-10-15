#ifndef INNERCUTFINDER_H
#define INNERCUTFINDER_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace cv;

#define ENABLE_LOG 1
#if ENABLE_LOG
#define LOG(x) std::cout << x
#define LOGLN(x) std::cout << x << std::endl
#else
#define LOG(x)
#define LOGLN(x)
#endif

class InnerCutFinder
{
public:
    InnerCutFinder(InputArray mask);
    Rect getROI();
    void process();

private:
    Rect processFirst();
    bool processLevel(int level);
    std::vector<Mat> pyramid;
    std::vector<Rect> pyramid_roi;
    int min_search_res;
    float step_down_scale;
    float ratio_x, ratio_y;
    float roi_min_area;
    bool done, failed;
};

#endif // INNERCUTFINDER_H
