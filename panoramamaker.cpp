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
    scale(1),
    stitcher(Stitcher::createDefault(try_use_gpu))
{}

void PanoramaMaker::run() {
    int N = images_path.size();

    for (int i=0; i<N; ++i) {
        Mat image = imread(images_path[i].toUtf8().constData());
        resize(image, image, Size(0,0), scale, scale);
        images.push_back(image);
        emit percentage(10*((i+1.0)/N));
    }

    Mat pano;
    Stitcher::Status status;

    status = stitcher.estimateTransform(images);
    if (status != Stitcher::OK) {
        emit failed();
    } else {
        emit percentage(50);
    }

    status = stitcher.composePanorama(pano);
    if (status != Stitcher::OK) {
        emit failed();
    } else {
        emit percentage(90);
    }

    if (status != Stitcher::OK) {
        qDebug() << "Can't stitch images, error code = " << int(status);
        emit failed();
    } else {
        qDebug() << "Stiching worked !";
        string out = output_fileinfo.absoluteFilePath().toUtf8().constData();
        qDebug() << "Writing to " << QString::fromStdString(out);
        imwrite(out, pano);
    }
    emit percentage(100);
}

void PanoramaMaker::setImages(QStringList files, QString output_filepath) {
    images_path = files;
    output_fileinfo = QFileInfo(output_filepath);
}

void PanoramaMaker::setWarpMode(QString mode) {
    WarpMode n_mode;
    if (mode == QString("Plane")) {
        n_mode = Plane;
    } else if (mode == QString("Cylindrical")) {
        n_mode = Cylindrical;
    } else if (mode == QString("Spherical")) {
        n_mode = Spherical;
    } else {
        return;
    }
    setWarpMode(n_mode);
}

void PanoramaMaker::setWarpMode(WarpMode mode) {
    Ptr<WarperCreator> warper;
    switch(mode) {
    case Plane:
        warper = makePtr<PlaneWarper>();
        break;
    case Cylindrical:
        warper = makePtr<CylindricalWarper>();
        break;
    case Spherical:
        warper = makePtr<SphericalWarper>();
        break;
    }
    stitcher.setWarper(warper);
}

void PanoramaMaker::setDownscale(double scale) {
    this->scale = scale;
}
