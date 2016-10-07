#ifndef PANORAMAMAKER_H
#define PANORAMAMAKER_H

#include <opencv2/stitching.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include <QThread>
#include <QFileInfo>
#include <vector>
#include <QProgressBar>

using namespace cv;
using namespace std;

class PanoramaMaker : public QThread
{
    Q_OBJECT
public:
    explicit PanoramaMaker(QObject *parent = 0);
    void setImages(QStringList files,
                   QString output_filename,
                   QString output_ext);

    void setWarpMode(QString mode);
    void setSeamFinderMode(QString mode);
    void setBlenderMode(QString mode, double param=-1);
    void setExposureCompensatorMode(QString mode, double bs=32);
    void setBundleAdjusterMode(QString mode);
    void setFeaturesFinderMode(QString mode);
    void setFeaturesMatchingMode(QString mode, double param=0.65);
    void setAssociatedProgressBar(QProgressBar *pb) { progress_bar = pb; }

    QProgressBar* getAssociatedProgresBar() { return progress_bar; }

    void unsafeRun();
    void run();

    QString getStitcherConfString();
    Stitcher* get_stitcher() { return &stitcher; }
    QString get_output_filename() { return output_filename; }

private:
    void fail(Stitcher::Status status);
    void fail(QString msg=QString("Unknown error"));

    bool try_use_gpu;
    QStringList images_path;
    QString output_filename, output_ext;

    Stitcher stitcher;
    Stitcher::Status status;
    vector<Mat> images;
    Mat pano;

    QString seam_finder_mode, warp_mode, blender_mode, exposure_compensator_mode;
    QString bundle_adjuster_mode, features_finder_mode, features_matcher_mode;
    double blender_param, features_matcher_param;

    QProgressBar *progress_bar;

signals:
    void percentage(int);
    void failed(QString msg=QString());

public slots:
};

#endif // PANORAMAMAKER_H
