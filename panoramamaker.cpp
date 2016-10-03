#include "panoramamaker.h"

#include <opencv2/stitching.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include <unistd.h>

using namespace cv;
using namespace std;

PanoramaMaker::PanoramaMaker(QObject *parent) :
    QThread(parent),
    try_use_gpu(true),
    stitcher(Stitcher::createDefault(try_use_gpu))
{}

void PanoramaMaker::run() {
    int N = images_path.size();

    for (int i=0; i<N; ++i) {
        Mat image = imread(images_path[i].toUtf8().constData());
        images.push_back(image);
        emit percentage(10*((i+1)/N));
    }

    Mat pano;
    Stitcher::Status status = stitcher.stitch(images, pano);
    emit percentage(90);

    if (status != Stitcher::OK) {
        qDebug() << "Can't stitch images, error code = " << int(status);
    } else {
        qDebug() << "Stiching worked !";
        string out = output_fileinfo.absoluteFilePath().toUtf8().constData();
        qDebug() << "Writing to " << QString::fromStdString(out);
        imwrite(out, pano);
    }
    emit percentage(100);
    emit done();
}

void PanoramaMaker::setImages(QStringList files, QString output_filepath) {
    images_path = files;
    output_fileinfo = QFileInfo(output_filepath);
}
