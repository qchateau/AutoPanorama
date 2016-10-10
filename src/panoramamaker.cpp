#include "panoramamaker.h"
#include "akazefeaturesfinder.h"

#include <opencv2/stitching.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/ocl.hpp>

#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <unistd.h>
#include <exception>
#include <typeinfo>

using namespace cv;
using namespace std;

static Stitcher unique_stitcher = Stitcher::createDefault(false);

PanoramaMaker::PanoramaMaker(QObject *parent) :
    QThread(parent),
    stitcher(unique_stitcher),
    status(STOPPED),
    total_time(-1),
    progress(0),
    try_use_gpu(false)
{
}

void PanoramaMaker::run() {
    if (!configureStitcher())
    {
        fail("Configuration error");
        return;
    }
    try {
        status = WORKING;
        timer.start();
        unsafeRun();
        total_time = timer.elapsed();
        status = DONE;
        emit done();
    }
    catch (cv::Exception& e) {
        qDebug() << "OpenCV error during stitching : " << QString(e.what());
        fail("OpenCV error during stitching");
    }
    catch(std::bad_alloc& e) {
        fail("Bad alloc error");
        qDebug() << "Bad alloc error : " << QString(e.what());
    }
    catch(...) {
        fail();
        qDebug() << "setUnknown exception";
    }
}

void PanoramaMaker::unsafeRun() {
    int N = images_path.size();

    vector<Mat> images;
    for (int i=0; i<N; ++i) {
        Mat image = imread(images_path[i].toUtf8().constData());
        images.push_back(image);
        setProgress(10*((i+1.0)/N));
    }

    Stitcher::Status stitcher_status;
    stitcher_status = stitcher.estimateTransform(images);
    if (stitcher_status != Stitcher::OK) {
        fail(stitcher_status);
        return;
    } else {
        setProgress(30);
    }

    Mat pano;
    stitcher_status = stitcher.composePanorama(pano);
    if (stitcher_status != Stitcher::OK) {
        fail(stitcher_status);
        return;
    } else {
        setProgress(90);
    }

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
    setProgress(100);
}

void PanoramaMaker::fail(Stitcher::Status status) {
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
    fail(msg);
}

void PanoramaMaker::fail(QString msg) {
    emit failed(msg);
    status = FAILED;
}

void PanoramaMaker::setImages(QStringList files, QString output_filename, QString output_ext) {
    images_path = files;
    this->output_filename = output_filename;
    this->output_ext = output_ext;
}

void PanoramaMaker::setWarpMode(QString mode) {
    warp_mode = mode;
}

void PanoramaMaker::setSeamFinderMode(QString mode) {
    seam_finder_mode = mode;
}

void PanoramaMaker::setBlenderMode(QString mode, double param) {
    if (mode == QString("Feather")) {
        if (param < 0) {
            param = 0.02f;
        }
    } else if (mode == QString("Multiband")) {
        if (param < 0) {
            param = 5;
        }
    }
    blender_mode = mode;
    blender_param = param;
}

void PanoramaMaker::setExposureCompensatorMode(QString mode, double bs) {
    exposure_compensator_param = bs;
    exposure_compensator_mode = mode;
}

void PanoramaMaker::setBundleAdjusterMode(QString mode) {
    bundle_adjuster_mode = mode;
}

void PanoramaMaker::setFeaturesFinderMode(QString mode) {
    features_finder_mode = mode;
}

void PanoramaMaker::setFeaturesMatchingMode(QString mode, double param) {
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

    if (total_time > 0)
    {
        conf += QString("Total time : %1s").arg(round(total_time/1000.));
        conf += QString("\n\n");
    }

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
    if (exposure_compensator_mode == QString("Blocks Gain") ||
        exposure_compensator_mode == QString("Blocks BGR") ||
        exposure_compensator_mode == QString("Blocks SV"))
    {
        conf += QString("\n");
        conf += QString("Exposure compensator blocks size : %1").arg(exposure_compensator_param);
    }
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
    conf += QString("\n\n");

    conf += QString("Try to use GPU : %1").arg(try_use_gpu ? "Yes" : "No");

    return conf;
}

void PanoramaMaker::setProgress(int prog)
{
    progress = prog;
    emit percentage(prog);
}

bool PanoramaMaker::configureStitcher()
{
    ocl::setUseOpenCL(try_use_gpu);

    // Warper
    Ptr<WarperCreator> warper;
    if (warp_mode == QString("Plane")) {
        warper = makePtr<PlaneWarper>();
    } else if (warp_mode == QString("Cylindrical")) {
        warper = makePtr<CylindricalWarper>();
    } else if (warp_mode == QString("Spherical")) {
        warper = makePtr<SphericalWarper>();
    } else {
        return false;
    }
    stitcher.setWarper(warper);

    // Seam finder
    Ptr<detail::SeamFinder> seamfinder;
    if (seam_finder_mode == QString("None")) {
        seamfinder = makePtr<detail::NoSeamFinder>();
    } else if (seam_finder_mode == QString("Voronoi")) {
        seamfinder = makePtr<detail::VoronoiSeamFinder>();
    } else if (seam_finder_mode == QString("Graph cut color")) {
        seamfinder = makePtr<detail::GraphCutSeamFinder>(detail::GraphCutSeamFinderBase::COST_COLOR);
    } else if (seam_finder_mode == QString("Graph cut gradient")) {
        seamfinder = makePtr<detail::GraphCutSeamFinder>(detail::GraphCutSeamFinderBase::COST_COLOR_GRAD);
    } else {
        return false;
    }
    stitcher.setSeamFinder(seamfinder);

    // Blender
    Ptr<detail::Blender> blender;
    if (blender_mode == QString("Feather")) {
        blender = makePtr<detail::FeatherBlender>(blender_param);
    } else if (blender_mode == QString("Multiband")) {
        blender = makePtr<detail::MultiBandBlender>(try_use_gpu, blender_param);
    } else {
        return false;
    }
    stitcher.setBlender(blender);

    // Exposure
    Ptr<detail::ExposureCompensator> exp_comp;
    if (exposure_compensator_mode == QString("None")) {
        exp_comp = makePtr<detail::NoExposureCompensator>();
    } else if (exposure_compensator_mode == QString("Gain")) {
        exp_comp = makePtr<detail::GainCompensator>();
    } else if (exposure_compensator_mode == QString("Blocks Gain")) {
        int bs = exposure_compensator_param;
        exp_comp = makePtr<detail::BlocksGainCompensator>(bs, bs);
    } else if (exposure_compensator_mode == QString("BGR")) {
        exp_comp = makePtr<detail::ChannelsCompensator>(detail::ChannelsCompensator::BGR);
    } else if (exposure_compensator_mode == QString("SV")) {
        exp_comp = makePtr<detail::ChannelsCompensator>(detail::ChannelsCompensator::SV);
    } else if (exposure_compensator_mode == QString("Blocks BGR")) {
        int bs = exposure_compensator_param;
        exp_comp = makePtr<detail::BlocksChannelsCompensator>(detail::ChannelsCompensator::BGR, bs, bs);
    } else if (exposure_compensator_mode == QString("Blocks SV")) {
        int bs = exposure_compensator_param;
        exp_comp = makePtr<detail::BlocksChannelsCompensator>(detail::ChannelsCompensator::SV, bs, bs);
    } else {
        return false;
    }
    stitcher.setExposureCompensator(exp_comp);

    // Bundle adjuster
    Ptr<detail::BundleAdjusterBase> bundle_adj;
    if (bundle_adjuster_mode == QString("Ray")) {
        bundle_adj = makePtr<detail::BundleAdjusterRay>();
    } else if (bundle_adjuster_mode == QString("Reproj")) {
        bundle_adj = makePtr<detail::BundleAdjusterReproj>();
    } else {
        return false;
    }
    stitcher.setBundleAdjuster(bundle_adj);

    // Features finder
    Ptr<detail::FeaturesFinder> ffinder;
    if (features_finder_mode == QString("ORB")) {
        ffinder = makePtr<detail::OrbFeaturesFinder>();
    } else if (features_finder_mode == QString("AKAZE")) {
        ffinder = makePtr<detail::AKAZEFeaturesFinder>();
    } else {
        return false;
    }
    stitcher.setFeaturesFinder(ffinder);

    // Matcher
    Ptr<detail::FeaturesMatcher> matcher;
    if (features_matcher_mode == QString("Best of 2 nearest")) {
        matcher = makePtr<detail::BestOf2NearestMatcher>(try_use_gpu, features_matcher_param);
    } else {
        return false;
    }
    stitcher.setFeaturesMatcher(matcher);

    return true;
}
