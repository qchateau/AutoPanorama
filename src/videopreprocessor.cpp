#include "videopreprocessor.h"
#include <QtDebug>

using namespace  cv;

VideoPreprocessor::VideoPreprocessor(const std::string& video_path) :
    capture(video_path)
{
    if (!capture.isOpened())
        throw std::invalid_argument("File "+video_path+" is not supported");
    frame_count = capture.get(CAP_PROP_FRAME_COUNT);
}

std::vector<Mat> VideoPreprocessor::evenTimeSpace(int nr)
{
    std::vector<Mat> images;

    double interval;
    if (nr > 1 && frame_count > 1)
        interval = (frame_count-1) / (nr-1);
    else
        interval = frame_count;
    interval = interval >= 1 ? interval : 1;

    for (double idx=0; idx < frame_count; idx+=interval)
    {
        int nr_try = 0;
        Mat img;
        capture.set(CAP_PROP_POS_FRAMES, std::round(idx));
        while (img.empty() && nr_try++ < interval)
            capture.read(img);
        if (img.empty())
            qDebug() << "Could not find a non empty image at" << idx << ": skipping";
        else
            images.push_back(img);
    }
    return images;
}
