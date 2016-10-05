#include "akazefeaturesfinder.h"

#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/stitching/detail/matchers.hpp>
#include <QtDebug>

AKAZEFeaturesFinder::AKAZEFeaturesFinder() {
    qDebug() << "Creating akaze";
    akaze = AKAZE::create();
    qDebug() << "Created akaze";
}

void AKAZEFeaturesFinder::find(InputArray image, detail::ImageFeatures &features) {
    CV_Assert((image.type() == CV_8UC3) || (image.type() == CV_8UC1));
    Mat desc;
    UMat uimg = image.getUMat();
    akaze->detectAndCompute(uimg, UMat(), features.keypoints, desc);
    features.descriptors = desc.getUMat(ACCESS_RW);
}
