#ifndef AKAZEFEATURESFINDER_H
#define AKAZEFEATURESFINDER_H

#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/stitching/detail/matchers.hpp>
#include <opencv2/opencv_modules.hpp>
#include <vector>

using namespace cv;
using namespace std;

class AKAZEFeaturesFinder : public detail::FeaturesFinder {
public:
    AKAZEFeaturesFinder();

private:
    void find(InputArray image, detail::ImageFeatures &features);

    Ptr<AKAZE> akaze;
};

#endif // AKAZEFEATURESFINDER_H
