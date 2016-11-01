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
    LOGLN("  level " << level+1);
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

        bool max_side[4] = {false, false, false, false};
        Point2f best_tl, best_br;
        Mat mask = pyramid[level];
        Rect best_roi = roi;

        best_tl = tl;
        best_br = br;

        int inc = 0;
        while (!(max_side[0] && max_side[1] && max_side[2] && max_side[3]))
        {
            ++inc;
            for (int s = 0; s < 4; ++s)
            {
                if (max_side[s])
                    continue;

                Point2f new_tl = best_tl;
                Point2f new_br = best_br;

                if (s == 0) // top
                    new_tl -= Point2f(0, y_inc*inc);
                else if (s == 1) // right
                    new_br += Point2f(x_inc*inc, 0);
                else if (s == 2) // bottom
                    new_br += Point2f(0, y_inc*inc);
                else if (s == 3) // left
                    new_tl -= Point2f(x_inc*inc, 0);

                Rect new_roi = Rect(Point(new_tl), Point(new_br));
                if (new_roi.tl() == best_roi.tl() && new_roi.br() == best_roi.br())
                    continue;
                if (new_roi.x+new_roi.width > mask.cols ||
                        new_roi.x < 0 ||
                        new_roi.y+new_roi.height > mask.rows ||
                        new_roi.y < 0 ||
                        countNonZero(mask(new_roi)) != long(new_roi.area()))
                {
                    max_side[s] = true;
                }
                else
                {
                    best_tl = new_tl;
                    best_br = new_br;
                    best_roi = new_roi;
                }
            }
        }

        LOGLN("  ROI grew " << best_roi.area() - roi.area() << " pixels over " << inc << " iterations");
        roi = best_roi;
    }

    bool ret = (roi.area() >= roi_min_area);
    if (level != 0)
        roi = Rect(roi.tl() + Point(1,1), roi.br() - Point(1,1));
    pyramid_roi[level] = roi;
    return ret;
}
