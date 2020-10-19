#ifndef VIDEOPREPROCESSOR_H
#define VIDEOPREPROCESSOR_H

#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

class VideoPreprocessor {
public:
    VideoPreprocessor(const std::string& video_path);

    std::vector<cv::Mat> evenTimeSpace(int nr);

private:
    cv::VideoCapture capture;
    double frame_count;
};

#endif // VIDEOPREPROCESSOR_H
