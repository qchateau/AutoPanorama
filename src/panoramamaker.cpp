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

PanoramaMaker::PanoramaMaker(QObject *parent) :
    QThread(parent),
    status(STOPPED),
    total_time(-1),
    progress(0),
    try_use_gpu(false)
{
}

void PanoramaMaker::run() {
    if (!configureStitcher())
        failed("Configuration error");
    else
    {
        try {
            status = WORKING;
            timer.start();
            Stitcher::Status stitcher_status = unsafeRun();
            total_time = timer.elapsed();
            if (stitcher_status == Stitcher::OK)
                done();
            else
                failed(stitcher_status);
        }
        catch (cv::Exception& e) {
            qDebug() << "OpenCV error during stitching : " << QString(e.what());
            failed("OpenCV error during stitching");
        }
        catch(std::bad_alloc& e) {
            failed("Bad alloc error");
            qDebug() << "Bad alloc error : " << QString(e.what());
        }
        catch(...) {
            failed();
            qDebug() << "setUnknown exception";
        }
    }
    clean();
}

Stitcher::Status PanoramaMaker::unsafeRun() {
    int N = images_path.size();

    vector<Mat> images;
    for (int i=0; i<N; ++i) {
        Mat image = imread(images_path[i].toUtf8().constData());
        images.push_back(image);
        setProgress(10*((i+1.0)/N));
    }

    Stitcher::Status stitcher_status;
    stitcher_status = stitcher->estimateTransform(images);
    if (stitcher_status != Stitcher::OK) {
        return stitcher_status;
    } else {
        setProgress(30);
    }

    Mat pano;
    stitcher_status = stitcher->composePanorama(pano);
    if (stitcher_status != Stitcher::OK) {
        return stitcher_status;
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
        output_fileinfo = QFileInfo(QDir(output_dir).filePath(final_filename));
    } while (output_fileinfo.exists());

    string out = output_fileinfo.absoluteFilePath().toUtf8().constData();
    qDebug() << "Writing to " << QString::fromStdString(out);
    imwrite(out, pano);
    setProgress(100);
    return stitcher_status;
}

void PanoramaMaker::failed(Stitcher::Status status) {
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
    failed(msg);
}

void PanoramaMaker::failed(QString msg) {
    status = FAILED;
    status_msg = msg;
    emit is_failed(msg);
}

void PanoramaMaker::done()
{
    status = DONE;
    status_msg = "Done";
    emit is_done();
}

void PanoramaMaker::clean()
{
    if (!stitcher.empty())
    {
        stitcher.release();
        qDebug() << "Released sticher. Stitcher empty : " << stitcher.empty();
    }
}

void PanoramaMaker::setImages(QStringList files, QString output_filename_,
                              QString output_ext_, QString output_dir_)
{
    images_path = files;
    output_filename = output_filename_;
    output_ext = output_ext_;
    output_dir = output_dir_;
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

    conf += QString("Registration Resolution : %1 Mpx").arg(getRegistrationResol());
    conf += QString("\n\n");

    conf += QString("Features finder : %1").arg(getFeaturesFinderMode());
    conf += QString("\n");
    conf += QString("Features matcher : %1").arg(getFeaturesMatchingMode().mode);
    conf += QString("\n");
    conf += QString("Features matcher confidence : %1").arg(getFeaturesMatchingMode().conf);
    conf += QString("\n\n");

    conf += QString("Warp Mode : %1").arg(getWarpMode());
    conf += QString("\n");
    conf += QString("Wave Correction : %1").arg(getWaveCorrectionMode());
    conf += QString("\n");
    conf += QString("Interpolation mode : %1").arg(getInterpolationMode());
    conf += QString("\n\n");

    conf += QString("Bundle adjuster : %1").arg(getBunderAdjusterMode());
    conf += QString("\n");
    conf += QString("Panorama Confidence threshold : %1").arg(getPanoConfidenceThresh());
    conf += QString("\n\n");

    conf += QString("Exposure compensator mode : %1").arg(getExposureCompensatorMode().mode);
    if (getExposureCompensatorMode().mode == QString("Blocks Gain") ||
        getExposureCompensatorMode().mode == QString("Blocks BGR") ||
        getExposureCompensatorMode().mode == QString("Combined Gain") ||
        getExposureCompensatorMode().mode == QString("Combined BGR"))
    {
        conf += QString("\n");
        conf += QString("Exposure compensator blocks size : %1").arg(getExposureCompensatorMode().block_size);
    }
    conf += QString("\n\n");

    conf += QString("Seam Finder : %1").arg(seam_finder_mode);
    conf += QString("\n");
    conf += QString("Seam Estimation Resolution : %1 Mpx").arg(getSeamEstimationResol());
    conf += QString("\n\n");

    conf += QString("Blender type : %1").arg(getBlenderMode().mode);
    conf += QString("\n");
    if (getBlenderMode().mode == QString("Feather")) {
        conf += QString("Blender sharpness : %1").arg(getBlenderMode().sharpness);
    } else if (getBlenderMode().mode == QString("Multiband")) {
        conf += QString("Blender bands : %1").arg(getBlenderMode().bands);
    }
    conf += QString("\n\n");

    conf += QString("Compositing Resolution : %1").arg(getCompositingResol() == Stitcher::ORIG_RESOL ?
                 "Original" :
                 QString("%1  Mpx").arg(getCompositingResol()));
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
    stitcher = createStitcher(try_use_gpu);
    if (stitcher.empty())
        return false;

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
    stitcher->setWarper(warper);

    // Interpolation
    int interp;
    if (interp_mode == QString("Nearest")) {
        interp = INTER_NEAREST;
    } else if (interp_mode == QString("Linear")) {
        interp = INTER_LINEAR;
    } else if (interp_mode == QString("Cubic")) {
        interp = INTER_CUBIC;
    } else if (interp_mode == QString("Lanczos4")) {
        interp = INTER_LANCZOS4;
    } else {
        return false;
    }
    stitcher->setInterpolation(interp);

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
    stitcher->setSeamFinder(seamfinder);
    stitcher->setSeamEstimationResol(seam_est_resol);

    // Blender
    Ptr<detail::Blender> blender;
    if (blender_mode.mode == QString("Feather")) {
        blender = makePtr<detail::FeatherBlender>(blender_mode.sharpness);
    } else if (blender_mode.mode == QString("Multiband")) {
        blender = makePtr<detail::MultiBandBlender>(try_use_gpu, blender_mode.bands, CV_32F);
        //blender = makePtr<detail::MultiBandBlender>(try_use_gpu, blender_mode.bands, CV_16S);
    } else if (blender_mode.mode == QString("None")) {
            blender = makePtr<detail::Blender>();
    } else {
        return false;
    }
    stitcher->setBlender(blender);

    // Exposure
    Ptr<detail::ExposureCompensator> exp_comp;
    if (exposure_compensator_mode.mode == QString("None")) {
        exp_comp = makePtr<detail::NoExposureCompensator>();
    } else if (exposure_compensator_mode.mode == QString("Gain")) {
        exp_comp = makePtr<detail::GainCompensator>();
    } else if (exposure_compensator_mode.mode == QString("Blocks Gain")) {
        int bs = exposure_compensator_mode.block_size;
        exp_comp = makePtr<detail::BlocksGainCompensator>(bs, bs);
    } else if (exposure_compensator_mode.mode == QString("Combined Gain")) {
        int bs = exposure_compensator_mode.block_size;
        exp_comp = makePtr<detail::CombinedGainCompensator>(bs, bs);
    } else if (exposure_compensator_mode.mode == QString("BGR")) {
        exp_comp = makePtr<detail::ChannelsCompensator>();
    } else if (exposure_compensator_mode.mode == QString("Blocks BGR")) {
        int bs = exposure_compensator_mode.block_size;
        exp_comp = makePtr<detail::BlocksChannelsCompensator>(bs, bs);
    } else if (exposure_compensator_mode.mode == QString("Combined BGR")) {
        int bs = exposure_compensator_mode.block_size;
        exp_comp = makePtr<detail::CombinedChannelsCompensator>(bs, bs);
    } else {
        return false;
    }
    stitcher->setExposureCompensator(exp_comp);

    // Bundle adjuster
    Ptr<detail::BundleAdjusterBase> bundle_adj;
    if (bundle_adjuster_mode == QString("Ray")) {
        bundle_adj = makePtr<detail::BundleAdjusterRay>();
    } else if (bundle_adjuster_mode == QString("Reproj")) {
        bundle_adj = makePtr<detail::BundleAdjusterReproj>();
    } else {
        return false;
    }
    stitcher->setBundleAdjuster(bundle_adj);

    // Features finder
    Ptr<detail::FeaturesFinder> ffinder;
    if (features_finder_mode == QString("ORB")) {
        ffinder = makePtr<detail::OrbFeaturesFinder>();
    } else if (features_finder_mode == QString("AKAZE")) {
        ffinder = makePtr<detail::AKAZEFeaturesFinder>();
    } else {
        return false;
    }
    stitcher->setFeaturesFinder(ffinder);

    // Matcher
    Ptr<detail::FeaturesMatcher> matcher;
    if (features_matching_mode.mode == QString("Best of 2 nearest")) {
        matcher = makePtr<detail::BestOf2NearestMatcher>(try_use_gpu, features_matching_mode.conf);
    } else {
        return false;
    }
    stitcher->setFeaturesMatcher(matcher);

    // Wave correction
    if (wave_correction_mode == QString("Horizontal")) {
        stitcher->setWaveCorrection(true);
        stitcher->setWaveCorrectKind(detail::WAVE_CORRECT_HORIZ);
    } else if (wave_correction_mode == QString("Vertical")) {
        stitcher->setWaveCorrection(true);
        stitcher->setWaveCorrectKind(detail::WAVE_CORRECT_VERT);
    } else {
        stitcher->setWaveCorrection(false);
    }

    // Pano conf
    stitcher->setPanoConfidenceThresh(pano_conf_threshold);

    // Registration
    stitcher->setRegistrationResol(registration_resol);

    // Compositing
    stitcher->setCompositingResol(compositing_resol);

    return true;
}
