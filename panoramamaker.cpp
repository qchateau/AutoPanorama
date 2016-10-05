#include "panoramamaker.h"

#include <opencv2/stitching.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <unistd.h>
#include <exception>
#include <typeinfo>

using namespace cv;
using namespace std;

PanoramaMaker::PanoramaMaker(QObject *parent) :
    QThread(parent),
    try_use_gpu(true),
    stitcher(Stitcher::createDefault(try_use_gpu))
{
}

void PanoramaMaker::run() {
    try {
        qDebug() << "Starting unsafe run";
        unsafeRun();
        qDebug() << "Unsafe run went well";
    }
    catch (cv::Exception& e) {
        qDebug() << "OpenCV error during stitching : " << QString(e.what());
    }
    catch(std::bad_alloc& e) {
        qDebug() << "Bad alloc error : " << QString(e.what());
    }
    catch(...) {
        qDebug() << "Unknown exception";
    }
}

void PanoramaMaker::unsafeRun() {
    int N = images_path.size();

    for (int i=0; i<N; ++i) {
        Mat image = imread(images_path[i].toUtf8().constData());
        images.push_back(image);
        emit percentage(10*((i+1.0)/N));
    }

    status = stitcher.estimateTransform(images);
    qDebug() << "estimateTransform done : " << ((status != Stitcher::OK) ? "KO" : "OK");
    if (status != Stitcher::OK) {
        fail(status);
        return;
    } else {
        emit percentage(30);
    }

    status = stitcher.composePanorama(pano);
    qDebug() << "composePanorama done : " << ((status != Stitcher::OK) ? "KO" : "OK");
    if (status != Stitcher::OK) {
        fail(status);
        return;
    } else {
        emit percentage(90);
    }

    qDebug() << "Stiching worked !";

    int nr = 0;
    QFileInfo output_fileinfo;
    do {
        QString final_filename = output_filename;
        if (nr++ > 0) {
            final_filename += "_"+QString::number(nr);
        }
        final_filename += output_ext;
        output_fileinfo = QFileInfo(QDir(QFileInfo(images_path[0]).absoluteDir()).filePath(final_filename));
    } while (output_fileinfo.exists());

    string out = output_fileinfo.absoluteFilePath().toUtf8().constData();
    qDebug() << "Writing to " << QString::fromStdString(out);
    imwrite(out, pano);
    emit percentage(100);
}

void PanoramaMaker::fail(int status) {
    QString msg;
    switch(status) {
    case Stitcher::ERR_NEED_MORE_IMGS:
        msg = "Need more images";
        break;
    case Stitcher::ERR_HOMOGRAPHY_EST_FAIL:
        msg = "Homography estimation failed";
        break;
    case Stitcher::ERR_CAMERA_PARAMS_ADJUST_FAIL:
        msg = "Camera parameters adjustment failed";
        break;
    default:
        msg = "Unknown error";
    }
    emit failed(msg);
}

void PanoramaMaker::setImages(QStringList files, QString output_filename, QString output_ext) {
    images_path = files;
    this->output_filename = output_filename;
    this->output_ext = output_ext;
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
    } else {
        return;
    }
    stitcher.setBlender(blender);
    blender_mode = mode;
    blender_param = param;
}

void PanoramaMaker::setExposureCompensatorMode(QString mode, double bs) {
    Ptr<detail::ExposureCompensator> exp_comp;
    if (mode == QString("None")) {
        exp_comp = makePtr<detail::NoExposureCompensator>();
    } else if (mode == QString("Gain")) {
        exp_comp = makePtr<detail::GainCompensator>();
    } else if (mode == QString("Blocks Gain")) {
        exp_comp = makePtr<detail::BlocksGainCompensator>(bs, bs);
    } else {
        return;
    }
    stitcher.setExposureCompensator(exp_comp);
    exposure_compensator_mode = mode;
}

void PanoramaMaker::setBundleAdjusterMode(QString mode) {
    Ptr<detail::BundleAdjusterBase> bundle_adj;
    if (mode == QString("Ray")) {
        bundle_adj = makePtr<detail::BundleAdjusterRay>();
    } else if (mode == QString("Reproj")) {
        bundle_adj = makePtr<detail::BundleAdjusterReproj>();
    } else {
        return;
    }
    stitcher.setBundleAdjuster(bundle_adj);
    bundle_adjuster_mode = mode;
}

void PanoramaMaker::setFeaturesFinderMode(QString mode) {
    Ptr<detail::FeaturesFinder> ffinder;
    if (mode == QString("ORB")) {
        ffinder = makePtr<detail::OrbFeaturesFinder>();
    } else if (mode == QString("SURF")) {
        ffinder = makePtr<detail::SurfFeaturesFinder>();
    } else {
        return;
    }
    stitcher.setFeaturesFinder(ffinder);
    features_finder_mode = mode;
}

void PanoramaMaker::setFeaturesMatchingMode(QString mode, double param) {
    Ptr<detail::FeaturesMatcher> matcher;
    if (mode == QString("Best of 2 nearest")) {
        matcher = makePtr<detail::BestOf2NearestMatcher>(try_use_gpu, param);
    } else {
        return;
    }
    stitcher.setFeaturesMatcher(matcher);
    features_matcher_mode = mode;
    features_matcher_param = param;
}

QString PanoramaMaker::getStitcherConfString() {
    QStringList files_filename;
    for (int i=0; i<images_path.size(); ++i) {
        files_filename << QFileInfo(images_path[i]).fileName();
    }

    QString conf;
    conf += QString("Images : ");
    conf += QString("\n");
    conf += files_filename.join(", ");
    conf += QString("\n\n");

    conf += QString("Registration Resolution : %1 Mpx").arg(stitcher.registrationResol());
    conf += QString("\n\n");

    conf += QString("Features finder : %1").arg(features_finder_mode);
    conf += QString("\n");
    conf += QString("Features matcher : %1").arg(features_matcher_mode);
    conf += QString("\n");
    conf += QString("Features matcher confidence : %1").arg(features_matcher_param);
    conf += QString("\n\n");

    conf += QString("Warp Mode : %1").arg(warp_mode);
    conf += QString("\n");
    conf += QString("Wave Correction : %1").arg(stitcher.waveCorrection() ?
                    stitcher.waveCorrectKind() == detail::WAVE_CORRECT_HORIZ ? "Horizontal" : "Vertical"
                    : "No");
    conf += QString("\n\n");

    conf += QString("Bundle adjuster : %1").arg(bundle_adjuster_mode);
    conf += QString("\n");
    conf += QString("Panorama Confidence threshold : %1").arg(stitcher.panoConfidenceThresh());
    conf += QString("\n\n");

    conf += QString("Exposure compensator mode : %1").arg(exposure_compensator_mode);
    conf += QString("\n\n");

    conf += QString("Seam Finder : %1").arg(seam_finder_mode);
    conf += QString("\n");
    conf += QString("Seam Estimation Resolution : %1 Mpx").arg(stitcher.seamEstimationResol());
    conf += QString("\n\n");

    conf += QString("Blender type : %1").arg(blender_mode);
    conf += QString("\n");
    if (blender_mode == QString("Feather")) {
        conf += QString("Blender sharpness : %1").arg(blender_param);
    } else if (blender_mode == QString("Multiband")) {
        conf += QString("Blender bands : %1").arg(int(blender_param));
    }
    conf += QString("\n\n");

    conf += QString("Compositing Resolution : %1").arg(stitcher.compositingResol() == Stitcher::ORIG_RESOL ?
                 "Original" :
                 QString("%1  Mpx").arg(stitcher.compositingResol()));


    return conf;
}
