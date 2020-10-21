#include "post_process.h"

#include "innercutfinder.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>

namespace autopanorama {

RescalableLabel::RescalableLabel(QPixmap pixmap, QWidget* parent)
    : QLabel(parent), original_{pixmap}
{
    setMinimumSize(640, 480);
    setScaledContents(false);
}

void RescalableLabel::setPixmap(QPixmap pixmap)
{
    original_ = pixmap;
    updatePixmap();
}

void RescalableLabel::resizeEvent(QResizeEvent*)
{
    updatePixmap();
}

void RescalableLabel::updatePixmap()
{
    QPixmap scaled = original_.scaled(
        this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QLabel::setPixmap(scaled);
}

PostProcess::PostProcess(const OutputFiles& outputs, QWidget* parent)
    : QDialog(parent), outputs_{outputs}, original_{outputs.output.absoluteFilePath()}
{
    cv::Mat mask = cv::imread(
        outputs.mask.absoluteFilePath().toStdString(), cv::IMREAD_GRAYSCALE);
    mask_scale_ = std::min(640.0 / mask.cols, 480.0 / mask.rows);
    cv::resize(
        mask,
        downscaled_mask_,
        cv::Size(0, 0),
        mask_scale_,
        mask_scale_,
        cv::INTER_NEAREST);

    label_angle_ = new QLabel("Angle: 0°");

    slider_angle_coarse_ = new QSlider(Qt::Horizontal, this);
    slider_angle_coarse_->setMinimum(-180);
    slider_angle_coarse_->setMaximum(180);
    slider_angle_coarse_->setValue(0);
    QHBoxLayout* slider_coarse_layout = new QHBoxLayout;
    QLabel* coarse_label = new QLabel("Coarse");
    coarse_label->setFixedHeight(60);
    slider_coarse_layout->addWidget(coarse_label);
    slider_coarse_layout->addWidget(slider_angle_coarse_);

    slider_angle_precise_ = new QSlider(Qt::Horizontal, this);
    slider_angle_precise_->setMinimum(-5 * kPreciseSliderScale);
    slider_angle_precise_->setMaximum(5 * kPreciseSliderScale);
    slider_angle_precise_->setValue(0);
    QHBoxLayout* slider_precise_layout = new QHBoxLayout;
    QLabel* precise_label = new QLabel("Precise");
    precise_label->setFixedHeight(60);
    slider_precise_layout->addWidget(precise_label);
    slider_precise_layout->addWidget(slider_angle_precise_);

    label_rotated_ = new RescalableLabel(original_, this);
    label_cut_ = new RescalableLabel(original_, this);
    QHBoxLayout* images_layout = new QHBoxLayout;
    images_layout->addWidget(label_rotated_);
    images_layout->addWidget(label_cut_);

    QDialogButtonBox* button_box = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label_angle_);
    layout->setAlignment(label_angle_, Qt::AlignCenter);
    layout->addLayout(slider_coarse_layout);
    layout->addLayout(slider_precise_layout);

    layout->addLayout(images_layout);
    layout->addWidget(button_box);

    connect(
        slider_angle_coarse_,
        &QSlider::valueChanged,
        this,
        &PostProcess::updateRotated);
    connect(
        slider_angle_precise_,
        &QSlider::valueChanged,
        this,
        &PostProcess::updateRotated);

    connect(button_box, &QDialogButtonBox::accepted, this, &PostProcess::accept);
    connect(button_box, &QDialogButtonBox::rejected, this, &PostProcess::reject);
    connect(this, &PostProcess::accepted, this, &PostProcess::onSave);

    setLayout(layout);
    setModal(false);
    setWindowTitle("Post-process");
    setAttribute(Qt::WA_DeleteOnClose);
    updateRotated();
}

double PostProcess::rotation() const
{
    return slider_angle_coarse_->value()
           + slider_angle_precise_->value() / kPreciseSliderScale;
}
QMatrix PostProcess::qtMatrix() const
{
    QMatrix q_rot_matrix;
    q_rot_matrix.rotate(rotation());
    return q_rot_matrix;
}

cv::Mat PostProcess::cvMatrix(cv::InputArray image_to_rotate) const
{
    QMatrix q_true_matrix = QPixmap::trueMatrix(
        qtMatrix(), image_to_rotate.cols(), image_to_rotate.rows());
    double data[] = {
        q_true_matrix.m11(),
        q_true_matrix.m21(),
        q_true_matrix.dx(),
        q_true_matrix.m12(),
        q_true_matrix.m22(),
        q_true_matrix.dy(),
    };
    return cv::Mat(2, 3, CV_64FC1, data).clone();
}

void PostProcess::updateRotated()
{
    QPixmap rotated = original_.transformed(qtMatrix());
    label_rotated_->setPixmap(rotated);
    QString text;
    text.sprintf("Angle: %.2f°", rotation());
    label_angle_->setText(text);

    cv::UMat rotated_mask;
    cv::Size size(rotated.width() * mask_scale_, rotated.height() * mask_scale_);
    cv::warpAffine(
        downscaled_mask_,
        rotated_mask,
        cvMatrix(downscaled_mask_),
        size,
        cv::INTER_NEAREST);

    // Find the ROI
    InnerCutFinder finder(rotated_mask);
    cv::Rect roi = finder.getROI();

    // Upscale the ROI
    roi.x /= mask_scale_;
    roi.width /= mask_scale_;
    roi.y /= mask_scale_;
    roi.height /= mask_scale_;

    QPixmap cropped = rotated.copy(QRect(roi.x, roi.y, roi.width, roi.height));
    label_cut_->setPixmap(cropped);
}

void PostProcess::onSave()
{
    qDebug() << "Reading original image and mask";
    cv::Mat mask = cv::imread(
        outputs_.mask.absoluteFilePath().toStdString(), cv::IMREAD_GRAYSCALE);
    cv::Mat original = cv::imread(
        outputs_.output.absoluteFilePath().toStdString());

    qDebug() << "Rotating mask and image";
    QPixmap qt_rotated = original_.transformed(qtMatrix());
    cv::Size size(qt_rotated.width(), qt_rotated.height());

    cv::UMat rotated, rotated_mask;
    cv::warpAffine(mask, rotated_mask, cvMatrix(mask), size, cv::INTER_NEAREST);
    cv::warpAffine(original, rotated, cvMatrix(original), size, cv::INTER_CUBIC);

    qDebug() << "Computing ROI";
    InnerCutFinder finder(rotated_mask);
    cv::Rect roi = finder.getROI();
    QString path = getPostProcessPath();
    qDebug() << "Writing post-process to " << path;
    cv::imwrite(path.toStdString(), rotated(roi));
}

QString PostProcess::getPostProcessPath() const
{
    QFileInfo output = outputs_.output;
    QString basename = output.completeBaseName();
    QString suffix = output.suffix();
    QDir dir = output.absoluteDir();

    QString final_basename;
    int nr = 0;
    do {
        final_basename = basename + "_post_process";
        if (nr++ > 0)
            final_basename += "_" + QString::number(nr);

        output = QFileInfo(dir.filePath(final_basename + "." + suffix));
    } while (output.exists());
    return output.absoluteFilePath();
}

} // autopanorama
