#include "panoramamaker.h"

#include <opencv2/stitching.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include <QtDebug>
#include <QFileInfo>
#include <unistd.h>

using namespace cv;

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
        imwrite(output_filename.toUtf8().constData(), pano);
    }
    emit percentage(100);
    emit done();
}

void PanoramaMaker::setImages(QStringList files) {
    images_path = files;
    output_filename = QString();
    for (int i=0; i<files.size(); ++i) {
        QFileInfo file(files[i]);
        output_filename += file.baseName();
        if (i < files.size()-1) {
            output_filename += "_";
        }
    }
    output_filename += ".png";
}
