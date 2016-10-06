#include "akazefeaturesfinder.h"

#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/stitching/detail/matchers.hpp>
#include <QtDebug>

namespace cv {
namespace detail {

AKAZEFeaturesFinder::AKAZEFeaturesFinder(int descriptor_type,
                                         int descriptor_size,
                                         int descriptor_channels,
                                         float threshold,
                                         int nOctaves,
                                         int nOctaveLayers,
                                         int diffusivity) {
    akaze = AKAZE::create(descriptor_type, descriptor_size, descriptor_channels,
                          threshold, nOctaves, nOctaveLayers, diffusivity);
}

void AKAZEFeaturesFinder::find(InputArray image, ImageFeatures &features) {
    CV_Assert((image.type() == CV_8UC3) || (image.type() == CV_8UC1));
    Mat desc;
    UMat uimg = image.getUMat();
    akaze->detectAndCompute(uimg, UMat(), features.keypoints, desc);
    features.descriptors = desc.getUMat(ACCESS_READ);
}

} // namespace detail
} // namespace cv
