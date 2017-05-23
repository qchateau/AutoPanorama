#include "videopreprocessor.h"

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
    for (double idx=0; idx < frame_count; idx+=interval)
    {
        Mat img;
        capture.set(CAP_PROP_POS_FRAMES, std::round(idx));
        capture.read(img);
        images.push_back(img);
    }
    return images;
}
