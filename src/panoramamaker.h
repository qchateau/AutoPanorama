#ifndef PANORAMAMAKER_H
#define PANORAMAMAKER_H

#include <opencv2/stitching.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include <QThread>
#include <QElapsedTimer>
#include <QFileInfo>

using namespace cv;
using namespace std;

class PanoramaMaker : public QThread
{
    Q_OBJECT
public:
    enum Status { STOPPED, WORKING, DONE, FAILED };
    struct BlenderMode { QString mode; double sharpness; int bands; };
    struct ExposureComensatorMode { QString mode; int block_size; };
    struct FeaturesMatchingMode { QString mode; double conf; };

    static QStringList getSupportedExtension();

    explicit PanoramaMaker(QObject *parent = 0);

    void setImages(QStringList files);
    void setOutput(QString output_filename_,
                   QString output_ext_,
                   QString output_dir_);

    void setUseOpenCL(bool use) { try_use_opencl = use; }
    bool getUseOpenCL() { return try_use_opencl; }

    void setGenerateInnerCut(bool gen) { generate_inner_cut = gen; }
    bool getGenerateInnerCut() { return generate_inner_cut; }

    void setBlenderMode(BlenderMode mode) { blender_mode = mode; }
    BlenderMode getBlenderMode() { return blender_mode; }

    void setExposureCompensatorMode(ExposureComensatorMode mode) { exposure_compensator_mode = mode; }
    ExposureComensatorMode getExposureCompensatorMode() { return exposure_compensator_mode; }

    void setFeaturesMatchingMode(FeaturesMatchingMode mode) { features_matching_mode = mode; }
    FeaturesMatchingMode getFeaturesMatchingMode() { return features_matching_mode; }

    void setWarpMode(QString mode) { warp_mode = mode; }
    QString getWarpMode() { return warp_mode; }

    void setSeamFinderMode(QString mode) { seam_finder_mode = mode; }
    QString getSeamFinderMode() { return seam_finder_mode; }

    void setBundleAdjusterMode(QString mode) { bundle_adjuster_mode = mode; }
    QString getBunderAdjusterMode() { return bundle_adjuster_mode; }

    void setFeaturesFinderMode(QString mode) { features_finder_mode = mode; }
    QString getFeaturesFinderMode() { return features_finder_mode; }

    void setWaveCorrectionMode(QString mode) { wave_correction_mode = mode; }
    QString getWaveCorrectionMode() { return wave_correction_mode; }

    void setInterpolationMode(QString mode) { interp_mode = mode; }
    QString getInterpolationMode() { return interp_mode; }

    void setRegistrationResol(double res) { registration_resol = res; }
    double getRegistrationResol() { return registration_resol; }

    void setSeamEstimationResol(double res) { seam_est_resol = res; }
    double getSeamEstimationResol() { return seam_est_resol; }

    void setCompositingResol(double res) { compositing_resol = res; }
    double getCompositingResol() { return compositing_resol; }

    void setPanoConfidenceThresh(double conf) { pano_conf_threshold = conf; }
    double getPanoConfidenceThresh() { return pano_conf_threshold; }

    float getTotalTime() { return total_time/1000.; }
    float getProcTime() { return proc_time/1000.; }
    Status getStatus() { return status; }
    QString getStatusMsg() { return status_msg; }
    int getProgress() { return progress; }

    QString getStitcherConfString();
    QString getOutputFilename() { return output_filename; }

    Stitcher::Status unsafeRun();
    void run();

private:
    void failed(Stitcher::Status status);
    void failed(QString msg=QString("Unknown error"));
    void done();
    void clean();
    void setProgress(int prog);
    bool configureStitcher();

    QFileInfo genOutputFileInfo();
    QFileInfo genInnerCutOutputFileInfo();

    QStringList images_path;
    QString output_filename, output_ext, output_dir;

    Ptr<Stitcher> stitcher;

    QString status_msg;
    Status status;

    QString seam_finder_mode, warp_mode, bundle_adjuster_mode, interp_mode;
    QString wave_correction_mode, features_finder_mode;
    ExposureComensatorMode exposure_compensator_mode;
    BlenderMode blender_mode;
    FeaturesMatchingMode features_matching_mode;

    double seam_est_resol, registration_resol, compositing_resol;
    double pano_conf_threshold;

    QElapsedTimer total_timer, proc_timer;
    long total_time, proc_time;
    int progress;
    bool try_use_cuda, try_use_opencl, generate_inner_cut;

signals:
    void percentage(int);
    void is_failed(QString msg=QString());
    void is_done();

public slots:
};

#endif // PANORAMAMAKER_H
