#ifndef POST_PROCESS_H
#define POST_PROCESS_H

#include "rescalable_label.h"
#include "ui_post_process.h"

#include <condition_variable>
#include <mutex>

#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QRadioButton>
#include <QSlider>
#include <QThread>

#include <opencv2/opencv.hpp>

namespace autopanorama {

class UpdaterThread : public QThread {
    Q_OBJECT
signals:
    void rotated(QPixmap);
    void cropped(QPixmap);

public:
    UpdaterThread(const QString& path, QObject* parent = nullptr);
    ~UpdaterThread();

    void setAngle(double angle);

private:
    void run() override;

    std::mutex mutex_;
    std::condition_variable cv_;
    double angle_;
    bool is_new_;

    QPixmap pixmap_;
    cv::UMat mask_;
    double mask_scale_;
};

class PostProcess : public QDialog {
    Q_OBJECT
public:
    PostProcess(const QString& output_path, QWidget* parent = nullptr);

private:
    static constexpr double kPreciseSliderScale = 100;

    double rotation() const;
    QMatrix qtMatrix() const;
    cv::Mat cvMatrix(cv::InputArray image_to_rotate) const;
    void updateRotated();
    void onSave();
    QString getPostProcessPath() const;

    Ui::PostProcess* ui_;

    UpdaterThread* updater_thread_;
    QString output_path_;
    QPixmap original_;
};

} // autopanorama

#endif // POST_PROCESS_H
