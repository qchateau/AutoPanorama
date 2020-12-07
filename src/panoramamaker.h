#ifndef PANORAMAMAKER_H
#define PANORAMAMAKER_H

#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>

#include <QElapsedTimer>
#include <QFileInfo>
#include <QThread>

namespace autopanorama {

class PanoramaMaker : public QThread {
    Q_OBJECT
signals:
    void percentage(int);
    void isFailed(QString msg);
    void isDone();

public:
    enum Status { STOPPED, WORKING, DONE, FAILED };
    struct BlenderMode {
        QString mode;
        double sharpness;
        int bands;
    };
    struct ExposureComensatorMode {
        QString mode;
        QString type;
        int block_size;
        int nfeed;
        double similarity_th;
    };
    struct FeaturesMatchingMode {
        QString mode;
        double conf;
    };

    static QStringList getSupportedImageExtensions();
    static QStringList getSupportedVideoExtensions();

    explicit PanoramaMaker(QObject* parent = 0);

    void setImages(QStringList files);
    void setVideos(QStringList files);
    void setOutput(QString output_filename, QString output_dir, bool overwrite);

    void setUseOpenCL(bool use) { try_use_opencl_ = use; }
    bool getUseOpenCL() { return try_use_opencl_; }

    void setBlenderMode(BlenderMode mode) { blender_mode_ = mode; }
    BlenderMode getBlenderMode() { return blender_mode_; }

    void setExposureCompensatorMode(ExposureComensatorMode mode)
    {
        exposure_compensator_mode_ = mode;
    }
    ExposureComensatorMode getExposureCompensatorMode()
    {
        return exposure_compensator_mode_;
    }

    void setFeaturesMatchingMode(FeaturesMatchingMode mode)
    {
        features_matching_mode_ = mode;
    }
    FeaturesMatchingMode getFeaturesMatchingMode()
    {
        return features_matching_mode_;
    }

    void setWarpMode(QString mode) { warp_mode_ = mode; }
    QString getWarpMode() { return warp_mode_; }

    void setSeamFinderMode(QString mode) { seam_finder_mode_ = mode; }
    QString getSeamFinderMode() { return seam_finder_mode_; }

    void setBundleAdjusterMode(QString mode) { bundle_adjuster_mode_ = mode; }
    QString getBunderAdjusterMode() { return bundle_adjuster_mode_; }

    void setFeaturesFinderMode(QString mode) { features_finder_mode_ = mode; }
    QString getFeaturesFinderMode() { return features_finder_mode_; }

    void setWaveCorrectionMode(QString mode) { wave_correction_mode_ = mode; }
    QString getWaveCorrectionMode() { return wave_correction_mode_; }

    void setInterpolationMode(QString mode) { interp_mode_ = mode; }
    QString getInterpolationMode() { return interp_mode_; }

    void setRegistrationResol(double res) { registration_resol_ = res; }
    double getRegistrationResol() { return registration_resol_; }

    void setSeamEstimationResol(double res) { seam_est_resol_ = res; }
    double getSeamEstimationResol() { return seam_est_resol_; }

    void setCompositingResol(double res) { compositing_resol_ = res; }
    double getCompositingResol() { return compositing_resol_; }

    void setPanoConfidenceThresh(double conf) { pano_conf_threshold_ = conf; }
    double getPanoConfidenceThresh() { return pano_conf_threshold_; }

    void setImagesPerVideo(int nr) { images_per_video_ = nr; }
    int getImagesPerVideo() { return images_per_video_; }

    float getTotalTime() { return total_time_ / 1000.; }
    float getProcTime() { return proc_time_ / 1000.; }
    Status getStatus() { return status_; }
    QString getStatusMsg() { return status_msg_; }
    int getProgress() { return progress_; }

    QString getStitcherConfString();
    QString getOutputFilename() { return output_filename_; }
    QString getOutputFilePath() { return output_path_; }

    cv::Stitcher::Status unsafeRun();
    void run();

private:
    void loadImages();
    void loadImagesFromImages();
    void loadImagesFromVideos();
    void loadVideo(const QString& path);
    void failed(cv::Stitcher::Status status);
    void failed(QString msg = QString("Unknown error"));
    void done();
    void clean();
    void setProgress(double prog);
    void incProgress(double inc);
    bool configureStitcher();

    QString genOutputFilePath();

    QStringList images_path_;
    QStringList videos_path_;
    QString output_filename_;
    QString output_dir_;
    QString output_path_;
    bool overwrite_output_;

    cv::Ptr<cv::Stitcher> stitcher_;
    std::vector<cv::Mat> images_;

    QString status_msg_;
    Status status_;

    QString seam_finder_mode_;
    QString warp_mode_;
    QString bundle_adjuster_mode_;
    QString interp_mode_;
    QString wave_correction_mode_;
    QString features_finder_mode_;
    ExposureComensatorMode exposure_compensator_mode_;
    BlenderMode blender_mode_;
    FeaturesMatchingMode features_matching_mode_;

    double seam_est_resol_;
    double registration_resol_;
    double compositing_resol_;
    double pano_conf_threshold_;

    QElapsedTimer total_timer_;
    QElapsedTimer proc_timer_;
    long total_time_;
    long proc_time_;
    double progress_;
    int images_per_video_;
    bool try_use_cuda_;
    bool try_use_opencl_;
};

} // autopanorama

#endif // PANORAMAMAKER_H
