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
    AKAZEFeaturesFinder(int descriptor_type = AKAZE::DESCRIPTOR_MLDB,
                        int descriptor_size = 0,
                        int descriptor_channels = 3,
                        float threshold = 0.001f,
                        int nOctaves = 4,
                        int nOctaveLayers = 4,
                        int diffusivity = KAZE::DIFF_PM_G2);

private:
    void find(InputArray image, detail::ImageFeatures &features);

    Ptr<AKAZE> akaze;
};

#endif // AKAZEFEATURESFINDER_H
