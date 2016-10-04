#include "panoramamaker.h"

#include <opencv2/stitching.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include <unistd.h>
#include <exception>
#include <typeinfo>

using namespace cv;
using namespace std;

PanoramaMaker::PanoramaMaker(QObject *parent) :
    QThread(parent),
    try_use_gpu(true),
    scale(1),
    stitcher(Stitcher::createDefault(try_use_gpu))
{
    setSeamFinderMode("Graph cut gradient");
    setWarpMode("Spherical");
    setBlenderMode("Multiband");
}

void PanoramaMaker::run() {
    try {
        unsafeRun();
    }
    catch (cv::Exception& e) {
        qDebug() << "OpenCV error during stitching : " << QString(e.what());
    }
}

void PanoramaMaker::unsafeRun() {
    int N = images_path.size();

    for (int i=0; i<N; ++i) {
        Mat image = imread(images_path[i].toUtf8().constData());
        resize(image, image, Size(0,0), scale, scale);
        images.push_back(image);
        emit percentage(10*((i+1.0)/N));
    }

    status = stitcher.estimateTransform(images);
    qDebug() << "estimateTransform done : " << status;
    if (status != Stitcher::OK) {
        emit failed();
        return;
    } else {
        emit percentage(50);
    }

    status = stitcher.composePanorama(pano);
    qDebug() << "composePanorama done : " << status;
    if (status != Stitcher::OK) {
        emit failed();
        return;
    } else {
        emit percentage(90);
    }

    qDebug() << "Stiching worked !";
    string out = output_fileinfo.absoluteFilePath().toUtf8().constData();
    qDebug() << "Writing to " << QString::fromStdString(out);
    imwrite(out, pano);
    emit percentage(100);
}

void PanoramaMaker::setImages(QStringList files, QString output_filepath) {
    images_path = files;
    output_fileinfo = QFileInfo(output_filepath);
}

void PanoramaMaker::setWarpMode(QString mode) {
    Ptr<WarperCreator> warper;
    if (mode == QString("Plane")) {
        warper = makePtr<PlaneWarper>();
    } else if (mode == QString("Cylindrical")) {
        warper = makePtr<CylindricalWarper>();
    } else if (mode == QString("Spherical")) {
        warper = makePtr<SphericalWarper>();
    } else {
        return;
    }
    stitcher.setWarper(warper);
    warp_mode = mode;
}

void PanoramaMaker::setSeamFinderMode(QString mode) {
    Ptr<detail::SeamFinder> seamfinder;
    if (mode == QString("None")) {
        seamfinder = makePtr<detail::NoSeamFinder>();
    } else if (mode == QString("Voronoi")) {
        seamfinder = makePtr<detail::VoronoiSeamFinder>();
    } else if (mode == QString("Graph cut color")) {
        seamfinder = makePtr<detail::GraphCutSeamFinder>(detail::GraphCutSeamFinderBase::COST_COLOR);
    } else if (mode == QString("Graph cut gradient")) {
        seamfinder = makePtr<detail::GraphCutSeamFinder>(detail::GraphCutSeamFinderBase::COST_COLOR_GRAD);
    } else {
        return;
    }
    stitcher.setSeamFinder(seamfinder);
    seam_finder_mode = mode;
}

void PanoramaMaker::setBlenderMode(QString mode, double param) {
    Ptr<detail::Blender> blender;
    if (mode == QString("Feather")) {
        if (param < 0) {
            param = 0.02f;
        }
        blender = makePtr<detail::FeatherBlender>(param);
    } else if (mode == QString("Multiband")) {
        if (param < 0) {
            param = 5;
        }
        blender = makePtr<detail::MultiBandBlender>(try_use_gpu, param);
    }
    stitcher.setBlender(blender);
    blender_mode = mode;
    blender_param = param;
}

void PanoramaMaker::setDownscale(double scale) {
    this->scale = scale;
}

QString PanoramaMaker::getStitcherConfString() {
    QString conf;
    conf += QString("Original Downscale : %1 %\n").arg(int(scale*100));
    conf += QString("Registration Resolution : %1 Mpx\n").arg(stitcher.registrationResol());
    conf += QString("Compositing Resolution : %1\n\n").arg(stitcher.compositingResol() == Stitcher::ORIG_RESOL ?
                 "Original" :
                 QString("%1  Mpx").arg(stitcher.compositingResol()));

    conf += QString("Seam Finder : %1\n").arg(seam_finder_mode);
    conf += QString("Seam Estimation Resolution : %1 Mpx\n\n").arg(stitcher.seamEstimationResol());

    conf += QString("Blender type : %1\n").arg(blender_mode);
    if (blender_mode == QString("Feather")) {
        conf += QString("Blender sharpness : %1\n\n").arg(blender_param);
    } else if (blender_mode == QString("Multiband")) {
        conf += QString("Blender bands : %1\n\n").arg(int(blender_param));
    }

    conf += QString("Panorama Confidence Thresh : %1\n\n").arg(stitcher.panoConfidenceThresh());

    conf += QString("Warp Mode : %1\n").arg(warp_mode);
    conf += QString("Wave Correction : %1\n").arg(stitcher.waveCorrection() ?
                    stitcher.waveCorrectKind() == detail::WAVE_CORRECT_HORIZ ? "Horizontal" : "Vertical"
                    : "No");
    return conf;
}
