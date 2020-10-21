#ifndef POST_PROCESS_H
#define POST_PROCESS_H

#include "types.h"

#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QSlider>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

namespace autopanorama {

class RescalableLabel : public QLabel {
public:
    RescalableLabel(QPixmap pixmap, QWidget* parent = nullptr);
    void setPixmap(QPixmap pixmap);

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    void updatePixmap();

    QPixmap original_;
};

class PostProcess : public QDialog {
    Q_OBJECT
public:
    PostProcess(const OutputFiles& outputs, QWidget* parent = nullptr);

private:
    static constexpr double kPreciseSliderScale = 100;

    double rotation() const;
    QMatrix qtMatrix() const;
    cv::Mat cvMatrix(cv::InputArray image_to_rotate) const;
    void updateRotated();
    void onSave();
    QString getPostProcessPath() const;

    QSlider* slider_angle_coarse_;
    QSlider* slider_angle_precise_;
    QLabel* label_angle_;
    RescalableLabel* label_rotated_;
    RescalableLabel* label_cut_;

    OutputFiles outputs_;
    QPixmap original_;
    cv::UMat downscaled_mask_;
    double mask_scale_;
};

} // autopanorama

#endif // POST_PROCESS_H
