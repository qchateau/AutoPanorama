#ifndef INNERCUTFINDER_H
#define INNERCUTFINDER_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace cv;

#define LOG(x) std::cout << x << std::endl

class InnerCutFinder
{
public:
    InnerCutFinder(InputArray mask);
    Rect getROI();
    void process();

private:
    Rect processFirst();
    bool processLevel(int level);
    std::vector<UMat> pyramid;
    std::vector<Rect> pyramid_roi;
    int min_search_res;
    float step_down_scale;
    float ratio_x, ratio_y;
    float roi_min_area;
    bool done, failed;
};

#endif // INNERCUTFINDER_H
