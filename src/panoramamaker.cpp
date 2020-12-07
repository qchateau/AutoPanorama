#include "panoramamaker.h"
#include "exposure_compensator.h"
#include "utils.h"
#include "videopreprocessor.h"

#include <opencv2/core/ocl.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>

#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QtDebug>
#include <exception>
#include <vector>

namespace autopanorama {

QStringList PanoramaMaker::getSupportedImageExtensions()
{
    QStringList supported_extensions;
    supported_extensions << "jpg";
    supported_extensions << "jpeg";
    supported_extensions << "png";
    supported_extensions << "bmp";
    supported_extensions << "tiff";
    supported_extensions << "tif";
    supported_extensions << "webp";
    return supported_extensions;
}

QStringList PanoramaMaker::getSupportedVideoExtensions()
{
    QStringList supported_extensions;
    supported_extensions << "avi";
    supported_extensions << "mpg";
    supported_extensions << "mpeg";
    supported_extensions << "mp4";
    supported_extensions << "mov";
    supported_extensions << "mkv";
    return supported_extensions;
}

PanoramaMaker::PanoramaMaker(QObject* parent)
    : QThread(parent),
      overwrite_output_(false),
      status_(STOPPED),
      total_time_(-1),
      proc_time_(-1),
      progress_(0),
      try_use_cuda_(false),
      try_use_opencl_(false)
{
}

void PanoramaMaker::setImages(QStringList files)
{
    QStringList supported = getSupportedImageExtensions();
    for (const QString& file : files) {
        QFileInfo info(file);
        if (!info.exists() && info.isFile())
            throw std::invalid_argument(
                "File " + file.toStdString() + " does not exists");
        QString ext = info.suffix().toLower();
        if (!supported.contains(ext))
            throw std::invalid_argument(
                "Extension '" + ext.toStdString() + "' is not supported");
    }
    images_path_ = files;
}

void PanoramaMaker::setVideos(QStringList files)
{
    QStringList supported = getSupportedVideoExtensions();
    for (const QString& file : files) {
        QFileInfo info(file);
        if (!info.exists() && info.isFile())
            throw std::invalid_argument(
                "File " + file.toStdString() + " does not exists");
        QString ext = info.suffix().toLower();
        if (!supported.contains(ext))
            throw std::invalid_argument(
                "Extension '" + ext.toStdString() + "' is not supported");
    }
    videos_path_ = files;
}

void PanoramaMaker::setOutput(QString output_filename, QString output_dir, bool overwrite)
{
    output_filename_ = output_filename;
    output_dir_ = output_dir;
    overwrite_output_ = overwrite;
}

QString PanoramaMaker::getStitcherConfString()
{
    QString conf;
    if (images_path_.size() > 0) {
        QStringList files_filename;
        for (const QString& path : images_path_)
            files_filename << QFileInfo(path).fileName();
        conf += QString("Images : ");
        conf += QString("\n");
        conf += files_filename.join(", ");
    }
    else if (videos_path_.size() > 0) {
        QStringList files_filename;
        for (const QString& path : videos_path_)
            files_filename << QFileInfo(path).fileName();
        conf += QString("Videos : ");
        conf += QString("\n");
        conf += files_filename.join(", ");
        conf += QString("\n");
        conf += QString("Images extracted : %1").arg(getImagesPerVideo());
    }
    conf += QString("\n\n");

    conf +=
        QString("Registration Resolution : %1 Mpx").arg(getRegistrationResol());
    conf += QString("\n\n");

    conf += QString("Features finder : %1").arg(getFeaturesFinderMode());
    conf += QString("\n");
    conf += QString("Features matcher : %1").arg(getFeaturesMatchingMode().mode);
    conf += QString("\n");
    conf += QString("Features matcher confidence : %1")
                .arg(getFeaturesMatchingMode().conf);
    conf += QString("\n\n");

    conf += QString("Warp Mode : %1").arg(getWarpMode());
    conf += QString("\n");
    conf += QString("Wave Correction : %1").arg(getWaveCorrectionMode());
    conf += QString("\n");
    conf += QString("Interpolation mode : %1").arg(getInterpolationMode());
    conf += QString("\n\n");

    conf += QString("Bundle adjuster : %1").arg(getBunderAdjusterMode());
    conf += QString("\n");
    conf += QString("Panorama Confidence threshold : %1")
                .arg(getPanoConfidenceThresh());
    conf += QString("\n\n");

    conf += QString("Exposure compensator mode : %1")
                .arg(getExposureCompensatorMode().mode);
    conf += QString("\n");
    conf += QString("Similarity threshold = %1")
                .arg(getExposureCompensatorMode().similarity_th);
    if (getExposureCompensatorMode().mode == QString("Simple")
        || getExposureCompensatorMode().mode == QString("Blocks")
        || getExposureCompensatorMode().mode == QString("Combined")) {
        if (getExposureCompensatorMode().type == cv::detail::GainCompensator::GAIN)
            conf += QString(" Gain");
        else if (getExposureCompensatorMode().type == cv::detail::GainCompensator::CHANNELS)
            conf += QString(" Channels");
        if (getExposureCompensatorMode().mode == QString("Blocks")
            || getExposureCompensatorMode().mode == QString("Combined")) {
            conf += QString("\n");
            conf += QString("Exposure compensator blocks size : %1")
                        .arg(getExposureCompensatorMode().block_size);
        }
    }
    conf += QString("\n\n");

    conf += QString("Seam Finder : %1").arg(seam_finder_mode_);
    conf += QString("\n");
    conf += QString("Seam Estimation Resolution : %1 Mpx")
                .arg(getSeamEstimationResol());
    conf += QString("\n\n");

    conf += QString("Blender type : %1").arg(getBlenderMode().mode);
    conf += QString("\n");
    if (getBlenderMode().mode == QString("Feather"))
        conf += QString("Blender sharpness : %1").arg(getBlenderMode().sharpness);
    else if (getBlenderMode().mode == QString("Multiband"))
        conf += QString("Blender bands : %1").arg(getBlenderMode().bands);
    conf += QString("\n\n");

    conf += QString("Compositing Resolution : %1")
                .arg(
                    getCompositingResol() == cv::Stitcher::ORIG_RESOL
                        ? "Original"
                        : QString("%1  Mpx").arg(getCompositingResol()));
    conf += QString("\n\n");

    conf += QString("Try to use OpenCL : %1").arg(try_use_opencl_ ? "Yes" : "No");

    return conf;
}

cv::Stitcher::Status PanoramaMaker::unsafeRun()
{
    cv::Stitcher::Status stitcher_status;

    loadImages();
    proc_timer_.start();

    stitcher_status = stitcher_->estimateTransform(images_);
    if (stitcher_status != cv::Stitcher::OK)
        return stitcher_status;
    else
        setProgress(30);

    cv::UMat pano, alpha_pano;

    stitcher_status = stitcher_->composePanorama(pano);
    if (stitcher_status != cv::Stitcher::OK)
        return stitcher_status;
    else
        setProgress(80);

    cv::cvtColor(pano, alpha_pano, cv::COLOR_BGR2BGRA);
    std::vector<cv::UMat> channels;
    cv::split(alpha_pano, channels);
    channels[3] = stitcher_->resultMask();
    cv::merge(channels, alpha_pano);

    output_path_ = genOutputFilePath();

    qDebug() << "Writing panorama to " << output_path_;
    imwrite(output_path_.toStdString(), alpha_pano);

    proc_time_ = proc_timer_.elapsed();
    setProgress(100);
    return stitcher_status;
}

void PanoramaMaker::run()
{
    if (images_path_.size() < 2 && videos_path_.size() < 1)
        failed("Need at least 2 images or 1 video");
    else if (!QDir(output_dir_).exists())
        failed("Destination directory doesn't exists");
    else if (!configureStitcher())
        failed("Configuration error");
    else {
        try {
            status_ = WORKING;
            total_timer_.start();
            cv::Stitcher::Status stitcher_status = unsafeRun();
            total_time_ = total_timer_.elapsed();
            if (stitcher_status == cv::Stitcher::OK)
                done();
            else
                failed(stitcher_status);
        }
        catch (cv::Exception& e) {
            qDebug() << "OpenCV error during stitching : " << QString(e.what());
            failed("OpenCV error during stitching");
        }
        catch (std::bad_alloc& e) {
            failed("Bad alloc error");
            qDebug() << "Bad alloc error : " << QString(e.what());
        }
        catch (std::exception& e) {
            failed(e.what());
            qDebug() << "Exception :" << e.what();
        }
        catch (...) {
            failed();
            qDebug() << "Unknown exception";
        }
    }
    clean();
}

void PanoramaMaker::loadImages()
{
    loadImagesFromImages();
    loadImagesFromVideos();
}

void PanoramaMaker::loadImagesFromImages()
{
    int N = images_path_.size() + videos_path_.size();

    for (int i = 0; i < images_path_.size(); ++i) {
        cv::Mat image = cv::imread(images_path_[i].toUtf8().constData());
        images_.push_back(image);
        incProgress(10.0 / N);
    }
}

void PanoramaMaker::loadImagesFromVideos()
{
    int N = images_path_.size() + videos_path_.size();

    for (int i = 0; i < videos_path_.size(); ++i) {
        loadVideo(videos_path_[i]);
        incProgress(10.0 / N);
    }
}

void PanoramaMaker::loadVideo(const QString& path)
{
    std::string filepath = path.toUtf8().constData();
    VideoPreprocessor preproc(filepath);
    std::vector<cv::Mat> video_images = preproc.evenTimeSpace(images_per_video_);
    for (const cv::Mat& image : video_images)
        images_.push_back(image);
}

void PanoramaMaker::failed(cv::Stitcher::Status status)
{
    QString msg;
    switch (status) {
    case cv::Stitcher::ERR_NEED_MORE_IMGS:
        msg = "Need more images";
        break;
    case cv::Stitcher::ERR_HOMOGRAPHY_EST_FAIL:
        msg = "Homography estimation failed";
        break;
    case cv::Stitcher::ERR_CAMERA_PARAMS_ADJUST_FAIL:
        msg = "Camera parameters adjustment failed";
        break;
    default:
        msg = "Unknown error";
    }
    failed(msg);
}

void PanoramaMaker::failed(QString msg)
{
    status_ = FAILED;
    status_msg_ = msg;
    isFailed(msg);
}

void PanoramaMaker::done()
{
    status_ = DONE;
    status_msg_ = "Done";
    isDone();
}

void PanoramaMaker::clean()
{
    if (!stitcher_.empty()) {
        stitcher_.release();
        qDebug() << "Released sticher. Stitcher empty : " << stitcher_.empty();
    }
}

void PanoramaMaker::setProgress(double prog)
{
    progress_ = prog;
    percentage(std::round(progress_));
}

void PanoramaMaker::incProgress(double inc)
{
    setProgress(progress_ + inc);
}

bool PanoramaMaker::configureStitcher()
{
    cv::ocl::setUseOpenCL(try_use_opencl_);

    stitcher_ = cv::Stitcher::create();
    if (stitcher_.empty())
        return false;

    // Warper
    cv::Ptr<cv::WarperCreator> warper;
    if (warp_mode_ == QString("Perspective"))
        warper = cv::makePtr<cv::PlaneWarper>();
    else if (warp_mode_ == QString("Cylindrical"))
        warper = cv::makePtr<cv::CylindricalWarper>();
    else if (warp_mode_ == QString("Spherical"))
        warper = cv::makePtr<cv::SphericalWarper>();
    else
        return false;

    stitcher_->setWarper(warper);

    // Interpolation
    cv::InterpolationFlags interp;
    if (interp_mode_ == QString("Nearest"))
        interp = cv::INTER_NEAREST;
    else if (interp_mode_ == QString("Linear"))
        interp = cv::INTER_LINEAR;
    else if (interp_mode_ == QString("Cubic"))
        interp = cv::INTER_CUBIC;
    else if (interp_mode_ == QString("Lanczos4"))
        interp = cv::INTER_LANCZOS4;
    else
        return false;

    stitcher_->setInterpolationFlags(interp);

    // Seam finder
    cv::Ptr<cv::detail::SeamFinder> seamfinder;
    if (seam_finder_mode_ == QString("None"))
        seamfinder = cv::makePtr<cv::detail::NoSeamFinder>();
    else if (seam_finder_mode_ == QString("Voronoi"))
        seamfinder = cv::makePtr<cv::detail::VoronoiSeamFinder>();
    else if (seam_finder_mode_ == QString("Graph cut color"))
        seamfinder = cv::makePtr<cv::detail::GraphCutSeamFinder>(
            cv::detail::GraphCutSeamFinderBase::COST_COLOR);
    else if (seam_finder_mode_ == QString("Graph cut gradient"))
        seamfinder = cv::makePtr<cv::detail::GraphCutSeamFinder>(
            cv::detail::GraphCutSeamFinderBase::COST_COLOR_GRAD);
    else
        return false;

    stitcher_->setSeamFinder(seamfinder);
    stitcher_->setSeamEstimationResol(seam_est_resol_);

    // Blender
    cv::Ptr<cv::detail::Blender> blender;
    if (blender_mode_.mode == QString("Feather"))
        blender = cv::makePtr<cv::detail::FeatherBlender>(blender_mode_.sharpness);
    else if (blender_mode_.mode == QString("Multiband"))
        blender = cv::makePtr<cv::detail::MultiBandBlender>(
            try_use_cuda_, blender_mode_.bands, CV_32F);
    // blender = makePtr<detail::MultiBandBlender>(try_use_cuda, blender_mode.bands, CV_16S);
    else if (blender_mode_.mode == QString("None"))
        blender = cv::makePtr<cv::detail::Blender>();
    else
        return false;

    stitcher_->setBlender(blender);

    // Exposure
    cv::Ptr<cv::detail::ExposureCompensator> exp_comp;
    int bs = exposure_compensator_mode_.block_size;
    int nfeed = exposure_compensator_mode_.nfeed;
    double sim_th = exposure_compensator_mode_.similarity_th;
    if (exposure_compensator_mode_.mode == "None") {
        exp_comp = cv::makePtr<NoExposureCompensator>();
    }
    else if (exposure_compensator_mode_.mode == "Simple") {
        if (exposure_compensator_mode_.type == "Gain") {
            auto ptr = cv::makePtr<GainCompensator>(nfeed);
            ptr->setSimilarityThreshold(sim_th);
            exp_comp = ptr;
        }
        else if (exposure_compensator_mode_.type == "BGR") {
            auto ptr = cv::makePtr<ChannelsCompensator>(nfeed);
            ptr->setSimilarityThreshold(sim_th);
            exp_comp = ptr;
        }
        else {
            return false;
        }
    }
    else if (exposure_compensator_mode_.mode == "Blocks") {
        if (exposure_compensator_mode_.type == "Gain") {
            auto ptr = cv::makePtr<BlocksGainCompensator>(bs, bs, nfeed);
            ptr->setSimilarityThreshold(sim_th);
            exp_comp = ptr;
        }
        else if (exposure_compensator_mode_.type == "BGR") {
            auto ptr = cv::makePtr<BlocksChannelsCompensator>(bs, bs, nfeed);
            ptr->setSimilarityThreshold(sim_th);
            exp_comp = ptr;
        }
        else {
            return false;
        }
    }
    else if (exposure_compensator_mode_.mode == "Combined") {
        if (exposure_compensator_mode_.type == "Gain") {
            auto ptr = cv::makePtr<CombinedGainCompensator>(bs, bs, nfeed);
            ptr->setSimilarityThreshold(sim_th);
            exp_comp = ptr;
        }
        else if (exposure_compensator_mode_.type == "BGR") {
            auto ptr = cv::makePtr<CombinedChannelsCompensator>(bs, bs, nfeed);
            ptr->setSimilarityThreshold(sim_th);
            exp_comp = ptr;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }

    stitcher_->setExposureCompensator(exp_comp);

    // Bundle adjuster
    cv::Ptr<cv::detail::BundleAdjusterBase> bundle_adj;
    if (bundle_adjuster_mode_ == QString("Ray")) {
        bundle_adj = cv::makePtr<cv::detail::BundleAdjusterRay>();
    }
    else if (bundle_adjuster_mode_ == QString("Reproj")) {
        bundle_adj = cv::makePtr<cv::detail::BundleAdjusterReproj>();
    }
    else {
        return false;
    }
    stitcher_->setBundleAdjuster(bundle_adj);

    // Features finder
    cv::Ptr<cv::Feature2D> ffinder;
    if (features_finder_mode_ == QString("ORB"))
        ffinder = cv::ORB::create();
    else if (features_finder_mode_ == QString("AKAZE"))
        ffinder = cv::AKAZE::create();
    else
        return false;

    stitcher_->setFeaturesFinder(ffinder);

    // Matcher
    cv::Ptr<cv::detail::FeaturesMatcher> matcher;
    if (features_matching_mode_.mode == QString("Best of 2 nearest"))
        matcher = cv::makePtr<cv::detail::BestOf2NearestMatcher>(
            try_use_cuda_, features_matching_mode_.conf);
    else
        return false;

    stitcher_->setFeaturesMatcher(matcher);

    // Wave correction
    if (wave_correction_mode_ == QString("Horizontal")) {
        stitcher_->setWaveCorrection(true);
        stitcher_->setWaveCorrectKind(cv::detail::WAVE_CORRECT_HORIZ);
    }
    else if (wave_correction_mode_ == QString("Vertical")) {
        stitcher_->setWaveCorrection(true);
        stitcher_->setWaveCorrectKind(cv::detail::WAVE_CORRECT_VERT);
    }
    else if (wave_correction_mode_ == QString("Auto")) {
        stitcher_->setWaveCorrection(true);
        stitcher_->setWaveCorrectKind(cv::detail::WAVE_CORRECT_AUTO);
    }
    else {
        stitcher_->setWaveCorrection(false);
    }

    // Pano conf
    stitcher_->setPanoConfidenceThresh(pano_conf_threshold_);

    // Registration
    stitcher_->setRegistrationResol(registration_resol_);

    // Compositing
    stitcher_->setCompositingResol(compositing_resol_);

    return true;
}

QString PanoramaMaker::genOutputFilePath()
{
    QString filename = output_filename_ + ".png";
    if (overwrite_output_) {
        return QDir(output_dir_).absoluteFilePath(filename);
    }
    else {
        return generateNewFilename(filename, output_dir_);
    }
}

} // autopanorama
