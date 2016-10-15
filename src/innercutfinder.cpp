#include "innercutfinder.h"

InnerCutFinder::InnerCutFinder(InputArray mask) :
    min_search_res(5000),
    step_down_scale(0.5),
    done(false),
    failed(false)
{
    CV_Assert(mask.type() == CV_8U);

    LOGLN("Creating the mask pyramid ...");
    pyramid.push_back(mask.getMat());
    while (long(pyramid.back().total()) > min_search_res)
    {
        Mat down;
        resize(pyramid.back(), down, Size(0,0), step_down_scale, step_down_scale, INTER_NEAREST);
        pyramid.push_back(down);
    }
    pyramid_roi.resize(pyramid.size());
    roi_min_area = (min_search_res/2)*0.10;
    LOGLN("  pyramid has " << pyramid.size() << " levels.");
    LOGLN("  the smallest is " << pyramid.back().cols << "x" << pyramid.back().rows);
}

Rect InnerCutFinder::getROI()
{
    if (!done)
        process();
    if (failed)
        return Rect();
    else
        return pyramid_roi[0];
}

void InnerCutFinder::process()
{
    LOGLN("Processing pyramid ...");
#if ENABLE_LOG
    int64_t t = getTickCount();
#endif
    for (int level = pyramid.size()-1; level >= 0; --level)
        if (!processLevel(level))
        {
            failed = true;
            break;
        }
    done = true;
    LOGLN("Processing pyramid, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec");
}

Rect InnerCutFinder::processFirst()
{
    Rect best_roi;
    Size search_size = pyramid.back().size();
    for (int tl_x = 0; tl_x < search_size.width; ++tl_x)
        for (int tl_y = 0; tl_y < search_size.height; ++tl_y)
            for (int br_x = search_size.width; br_x > tl_x; --br_x)
                for (int br_y = search_size.height; br_y > tl_y; --br_y)
                {
                    Rect roi(Point(tl_x, tl_y), Point(br_x, br_y));
                    if (roi.area() < roi_min_area)
                        continue;
                    Mat pyr_roi = pyramid.back()(roi);
                    if (countNonZero(pyr_roi) == long(roi.area()) &&
                            roi.area() > best_roi.area())
                        best_roi = roi;
                }
    return best_roi;
}

bool InnerCutFinder::processLevel(int level)
{
    Rect roi;
    if (level == long(pyramid.size()-1))
    {
        roi = processFirst();
    }
    else
    {
        Rect down_roi = pyramid_roi[level+1];
        Point tl(down_roi.tl().x/step_down_scale, down_roi.tl().y/step_down_scale);
        Point br(down_roi.br().x/step_down_scale, down_roi.br().y/step_down_scale);
        roi = Rect(tl, br);

        float x_inc, y_inc;
        if (roi.width > roi.height)
        {
            x_inc = 1;
            y_inc = float(roi.height) / roi.width;
        }
        else
        {
            x_inc = float(roi.width) / roi.height;
            y_inc = 1;
        }

        Mat mask = pyramid[level];
        Rect best_roi = roi;
        for (int inc = 1;; ++inc)
        {
            Point new_tl = tl - Point(int(x_inc*inc), int(y_inc*inc));
            Point new_br = br + Point(int(x_inc*inc), int(y_inc*inc));
            Rect new_roi(new_tl, new_br);
            if (countNonZero(mask(new_roi)) != long(new_roi.area()))
                break;
            best_roi = new_roi;
        }
        roi = best_roi;
    }

    bool ret = (roi.area() >= roi_min_area);
    roi = Rect(roi.tl() + Point(1,1), roi.br() - Point(1,1));
    pyramid_roi[level] = roi;
    return ret;
}
