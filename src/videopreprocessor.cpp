#include "videopreprocessor.h"

#include <QtDebug>

namespace autopanorama {

VideoPreprocessor::VideoPreprocessor(const std::string& video_path)
    : capture_(video_path)
{
    if (!capture_.isOpened())
        throw std::invalid_argument("File " + video_path + " is not supported");
    frame_count_ = capture_.get(cv::CAP_PROP_FRAME_COUNT);
}

std::vector<cv::Mat> VideoPreprocessor::evenTimeSpace(int nr)
{
    std::vector<cv::Mat> images;

    double interval;
    if (nr > 1 && frame_count_ > 1)
        interval = (frame_count_ - 1) / (nr - 1);
    else
        interval = frame_count_;
    interval = interval >= 1 ? interval : 1;

    for (double idx = 0; idx < frame_count_; idx += interval) {
        int nr_try = 0;
        cv::Mat img;
        capture_.set(cv::CAP_PROP_POS_FRAMES, std::round(idx));
        while (img.empty() && nr_try++ < interval)
            capture_.read(img);
        if (img.empty())
            qInfo()
                << "Could not find a non empty image at" << idx << ": skipping";
        else
            images.push_back(img);
    }
    return images;
}

} // autopanorama
