#include "innercutfinder.h"

namespace autopanorama {
namespace {

bool isOutOfMask(cv::Mat_<uchar> mask)
{
    for (int y = 0; y < mask.rows; ++y) {
        for (int x = 0; x < mask.cols; ++x) {
            if (!mask(y, x)) {
                return true;
            }
        }
    }
    return false;
}

}

InnerCutFinder::InnerCutFinder(cv::InputArray mask)
    : min_search_res_(5000), step_down_scale_(0.5), done_(false), failed_(false)
{
    CV_Assert(mask.type() == CV_8U);
    pyramid_.push_back(mask.getMat());
}

cv::Rect InnerCutFinder::getROI()
{
    if (!done_)
        process();
    if (failed_)
        return cv::Rect();
    else
        return pyramid_roi_[0];
}

void InnerCutFinder::process()
{
#if ENABLE_LOG
    int64_t t0 = cv::getTickCount();
#endif
    LOGLN("Creating the mask pyramid ...");
    while (long(pyramid_.back().total()) > min_search_res_) {
        cv::Mat down;
        resize(
            pyramid_.back(),
            down,
            cv::Size(0, 0),
            step_down_scale_,
            step_down_scale_,
            cv::INTER_NEAREST);
        pyramid_.push_back(down);
    }
    pyramid_roi_.resize(pyramid_.size());
    roi_min_area_ = (min_search_res_ / 2) * 0.10;
    LOGLN("  pyramid has " << pyramid_.size() << " levels.");
    LOGLN(
        "  the smallest is " << pyramid_.back().cols << "x"
                             << pyramid_.back().rows);
    LOGLN(
        "Created pyramid, time: "
        << ((cv::getTickCount() - t0) / cv::getTickFrequency()) << " sec");

    LOGLN("Processing pyramid ...");
#if ENABLE_LOG
    int64_t t1 = cv::getTickCount();
#endif
    for (int level = static_cast<int>(pyramid_.size() - 1); level >= 0; --level)
        if (!processLevel(level)) {
            failed_ = true;
            break;
        }
    done_ = true;

    LOGLN(
        "Processing pyramid, time: "
        << ((cv::getTickCount() - t1) / cv::getTickFrequency()) << " sec");
}

cv::Rect InnerCutFinder::processFirst()
{
    cv::Rect best_roi;
    cv::Size search_size = pyramid_.back().size();
    for (int tl_x = 0; tl_x < search_size.width; ++tl_x)
        for (int tl_y = 0; tl_y < search_size.height; ++tl_y)
            for (int br_x = search_size.width; br_x > tl_x; --br_x)
                for (int br_y = search_size.height; br_y > tl_y; --br_y) {
                    cv::Rect roi(cv::Point(tl_x, tl_y), cv::Point(br_x, br_y));
                    if (roi.area() < roi_min_area_
                        || roi.area() < best_roi.area())
                        continue;
                    cv::Mat pyr_roi = pyramid_.back()(roi);
                    if (!isOutOfMask(pyr_roi))
                        best_roi = roi;
                }
    return best_roi;
}

bool InnerCutFinder::processLevel(int level)
{
#if ENABLE_LOG
    int64_t t0 = cv::getTickCount();
#endif
    LOGLN("  level " << level + 1);
    cv::Rect roi;
    if (level == long(pyramid_.size() - 1)) {
        roi = processFirst();
    }
    else {
        cv::Rect down_roi = pyramid_roi_[level + 1];
        cv::Point tl(
            static_cast<int>(down_roi.tl().x / step_down_scale_),
            static_cast<int>(down_roi.tl().y / step_down_scale_));
        cv::Point br(
            static_cast<int>(down_roi.br().x / step_down_scale_),
            static_cast<int>(down_roi.br().y / step_down_scale_));
        roi = cv::Rect(tl, br);

        float x_inc, y_inc;
        if (roi.width > roi.height) {
            x_inc = 1;
            y_inc = float(roi.height) / roi.width;
        }
        else {
            x_inc = float(roi.width) / roi.height;
            y_inc = 1;
        }

        bool max_side[4] = {false, false, false, false};
        cv::Point2f best_tl, best_br;
        cv::Mat mask = pyramid_[level];
        cv::Rect best_roi = roi;

        best_tl = tl;
        best_br = br;

        int inc = 0;
        while (!(max_side[0] && max_side[1] && max_side[2] && max_side[3])) {
            ++inc;
            for (int s = 0; s < 4; ++s) {
                if (max_side[s])
                    continue;

                cv::Point2f new_tl = best_tl;
                cv::Point2f new_br = best_br;
                cv::Rect grown_roi;

                if (s == 0) { // top
                    new_tl -= cv::Point2f(0, y_inc * inc);
                    grown_roi =
                        cv::Rect(new_tl, cv::Point2f(best_br.x, best_tl.y));
                }
                else if (s == 1) { // right
                    new_br += cv::Point2f(x_inc * inc, 0);
                    grown_roi =
                        cv::Rect(cv::Point2f(best_br.x, best_tl.y), new_br);
                }
                else if (s == 2) { // bottom
                    new_br += cv::Point2f(0, y_inc * inc);
                    grown_roi =
                        cv::Rect(cv::Point2f(best_tl.x, best_br.y), new_br);
                }
                else if (s == 3) { // left
                    new_tl -= cv::Point2f(x_inc * inc, 0);
                    grown_roi =
                        cv::Rect(new_tl, cv::Point2f(best_tl.x, best_br.y));
                }
                cv::Rect new_roi = cv::Rect(cv::Point(new_tl), cv::Point(new_br));

                cv::Rect(
                    std::min(new_roi.x, best_roi.x),
                    std::min(new_roi.y, best_roi.y),
                    new_roi.width > best_roi.width ? new_roi.width - best_roi.width
                                                   : new_roi.width,
                    new_roi.height > best_roi.height
                        ? new_roi.height - best_roi.height
                        : new_roi.height);

                if (new_roi.tl() == best_roi.tl() && new_roi.br() == best_roi.br())
                    continue;
                if (new_roi.x + new_roi.width > mask.cols || new_roi.x < 0
                    || new_roi.y + new_roi.height > mask.rows || new_roi.y < 0
                    || isOutOfMask(mask(grown_roi))) {
                    max_side[s] = true;
                }
                else {
                    best_tl = new_tl;
                    best_br = new_br;
                    best_roi = new_roi;
                }
            }
        }

        LOGLN(
            "  ROI grew " << best_roi.area() - roi.area() << " pixels over "
                          << inc << " iterations");
        roi = best_roi;
    }
    LOGLN(
        "  time: " << ((cv::getTickCount() - t0) / cv::getTickFrequency())
                   << " sec");

    bool ret = (roi.area() >= roi_min_area_);
    if (level != 0)
        roi = cv::Rect(roi.tl() + cv::Point(1, 1), roi.br() - cv::Point(1, 1));
    pyramid_roi_[level] = roi;
    return ret;
}

} // autopanorama
