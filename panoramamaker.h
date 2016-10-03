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
    void run();
    QFileInfo out_fileinfo() { return output_fileinfo; }

private:
    QStringList images_path;
    QFileInfo output_fileinfo;
    vector<Mat> images;

    bool try_use_gpu;
    Stitcher stitcher;

signals:
    void percentage(int);
    void done();

public slots:
};

#endif // PANORAMAMAKER_H
