#ifndef PANORAMAMAKER_H
#define PANORAMAMAKER_H

#include <opencv2/stitching.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include <QThread>
#include <vector>

using namespace cv;
using namespace std;

class PanoramaMaker : public QThread
{
    Q_OBJECT
public:
    explicit PanoramaMaker(QObject *parent = 0);
    void setImages(QStringList files);
    void run();
    QString out_filename() { return output_filename; }

private:
    QStringList images_path;
    QString output_filename;
    vector<Mat> images;

    bool try_use_gpu;
    Stitcher stitcher;

signals:
    void percentage(int);
    void done();

public slots:
};

#endif // PANORAMAMAKER_H
