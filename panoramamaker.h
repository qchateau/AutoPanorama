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

typedef enum WarpMode { Plane, Cylindrical, Spherical } WarpMode;

class PanoramaMaker : public QThread
{
    Q_OBJECT
public:
    explicit PanoramaMaker(QObject *parent = 0);
    void setImages(QStringList files,
                   QString output_filepath);

    void setWarpMode(QString mode);
    void setWarpMode(WarpMode mode);
    void setDownscale(double scale=1);
    void run();

    QFileInfo out_fileinfo() { return output_fileinfo; }
    Stitcher* get_stitcher() { return &stitcher; }

private:
    QStringList images_path;
    QFileInfo output_fileinfo;
    vector<Mat> images;

    bool try_use_gpu;
    double scale;
    Stitcher stitcher;

signals:
    void percentage(int);
    void failed(QString msg=QString());

public slots:
};

#endif // PANORAMAMAKER_H
