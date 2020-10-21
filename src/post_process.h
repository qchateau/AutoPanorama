#ifndef POST_PROCESS_H
#define POST_PROCESS_H

#include "rescalable_label.h"
#include "ui_post_process.h"

#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QRadioButton>
#include <QSlider>

#include <opencv2/opencv.hpp>

namespace autopanorama {

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

    QString output_path_;
    QPixmap original_;
    cv::UMat downscaled_mask_;
    double mask_scale_;
};

} // autopanorama

#endif // POST_PROCESS_H
