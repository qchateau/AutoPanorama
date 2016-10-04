#ifndef PANORAMAMAKER_H
#define PANORAMAMAKER_H

#include <opencv2/stitching.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include <QThread>
#include <QFileInfo>
#include <vector>

using namespace cv;
using namespace std;

class PanoramaMaker : public QThread
{
    Q_OBJECT
public:
    explicit PanoramaMaker(QObject *parent = 0);
    void setImages(QStringList files,
                   QString output_filepath);

    void setWarpMode(QString mode);
    void setSeamFinderMode(QString mode);
    void setBlenderMode(QString mode, double param=-1);

    void setDownscale(double scale=1);
    void unsafeRun();
    void run();

    QString getStitcherConfString();
    QFileInfo out_fileinfo() { return output_fileinfo; }
    Stitcher* get_stitcher() { return &stitcher; }

private:
    QStringList images_path;
    QFileInfo output_fileinfo;
    vector<Mat> images;

    bool try_use_gpu;
    double scale;
    Stitcher stitcher;
    Mat pano;
    Stitcher::Status status;

    QString seam_finder_mode, warp_mode, blender_mode;
    double blender_param;

signals:
    void percentage(int);
    void failed(QString msg=QString());

public slots:
};

#endif // PANORAMAMAKER_H
