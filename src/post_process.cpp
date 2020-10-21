#include "post_process.h"

#include "innercutfinder.h"
#include "utils.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QGraphicsColorizeEffect>
#include <QGridLayout>
#include <QVBoxLayout>

namespace autopanorama {
namespace {

QMatrix qtMatrix(double angle)
{
    QMatrix q_rot_matrix;
    q_rot_matrix.rotate(angle);
    return q_rot_matrix;
}

cv::Mat cvMatrix(double angle, cv::InputArray image_to_rotate)
{
    QMatrix q_true_matrix = QPixmap::trueMatrix(
        qtMatrix(angle), image_to_rotate.cols(), image_to_rotate.rows());
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

}

UpdaterThread::UpdaterThread(const QString& path, QObject* parent)
    : QThread(parent), is_new_{false}, pixmap_{path}
{
    cv::UMat original;
    cv::imread(path.toStdString(), cv::IMREAD_UNCHANGED).copyTo(original);
    std::vector<cv::UMat> channels;
    cv::split(original, channels);
    cv::UMat mask = channels.back();

    mask_scale_ = std::min(640.0 / mask.cols, 480.0 / mask.rows);
    cv::resize(
        mask, mask_, cv::Size(0, 0), mask_scale_, mask_scale_, cv::INTER_NEAREST);
}

UpdaterThread::~UpdaterThread()
{
    requestInterruption();
    cv_.notify_all();
    wait();
}

void UpdaterThread::setAngle(double angle)
{
    {
        std::unique_lock<std::mutex> lock{mutex_};
        angle_ = angle;
        is_new_ = true;
    }
    cv_.notify_one();
}

void UpdaterThread::run()
{
    while (!isInterruptionRequested()) {
        double angle;
        {
            std::unique_lock<std::mutex> lock{mutex_};
            cv_.wait(lock, [this]() {
                return isInterruptionRequested() || is_new_;
            });
            if (!is_new_) {
                continue;
            }

            angle = angle_;
            is_new_ = false;
        }

        QPixmap rotated = pixmap_.transformed(qtMatrix(angle));
        this->rotated(rotated);

        cv::UMat rotated_mask;
        cv::Size size(rotated.width() * mask_scale_, rotated.height() * mask_scale_);
        cv::warpAffine(
            mask_, rotated_mask, cvMatrix(angle, mask_), size, cv::INTER_NEAREST);

        // Find the ROI
        InnerCutFinder finder(rotated_mask);
        cv::Rect roi = finder.getROI();

        // Upscale the ROI
        roi.x /= mask_scale_;
        roi.width /= mask_scale_;
        roi.y /= mask_scale_;
        roi.height /= mask_scale_;

        QPixmap cropped = rotated.copy(QRect(roi.x, roi.y, roi.width, roi.height));
        this->cropped(cropped);
    }
}

PostProcess::PostProcess(const QString& output_path, QWidget* parent)
    : QDialog(parent),
      ui_(new Ui::PostProcess),
      output_path_{output_path},
      original_{output_path}
{
    auto set_disabled_effect = [](QWidget* widget) {
        QGraphicsColorizeEffect* disabled_effect = new QGraphicsColorizeEffect;
        disabled_effect->setColor(QColor(128, 128, 128));
        widget->setGraphicsEffect(disabled_effect);
    };
    auto clear_disabled_effect = [](QWidget* widget) {
        widget->setGraphicsEffect(nullptr);
    };

    updater_thread_ = new UpdaterThread(output_path_, this);
    updater_thread_->start();

    ui_->setupUi(this);

    ui_->slider_coarse->setMinimum(-180);
    ui_->slider_coarse->setMaximum(180);
    ui_->slider_coarse->setValue(0);

    ui_->slider_precise->setMinimum(-5 * kPreciseSliderScale);
    ui_->slider_precise->setMaximum(5 * kPreciseSliderScale);
    ui_->slider_precise->setValue(0);

    set_disabled_effect(ui_->label_cut);
    ui_->radio_rotated->setChecked(true);
    connect(ui_->label_rotated, &RescalableLabel::clicked, this, [=]() {
        ui_->radio_rotated->setChecked(true);
        clear_disabled_effect(ui_->label_rotated);
        set_disabled_effect(ui_->label_cut);
    });
    connect(ui_->label_cut, &RescalableLabel::clicked, this, [=]() {
        ui_->radio_cut->setChecked(true);
        clear_disabled_effect(ui_->label_cut);
        set_disabled_effect(ui_->label_rotated);
    });

    connect(
        ui_->slider_coarse,
        &QSlider::valueChanged,
        this,
        &PostProcess::updateRotated);
    connect(
        ui_->slider_precise,
        &QSlider::valueChanged,
        this,
        &PostProcess::updateRotated);

    connect(updater_thread_, &UpdaterThread::rotated, this, [this](QPixmap pixmap) {
        ui_->label_rotated->setPixmap(pixmap);
    });
    connect(updater_thread_, &UpdaterThread::cropped, this, [this](QPixmap pixmap) {
        ui_->label_cut->setPixmap(pixmap);
    });

    connect(this, &PostProcess::accepted, this, &PostProcess::onSave);

    setModal(false);
    setWindowTitle("Post-process");
    setAttribute(Qt::WA_DeleteOnClose);
    updateRotated();
}

double PostProcess::rotation() const
{
    return ui_->slider_coarse->value()
           + ui_->slider_precise->value() / kPreciseSliderScale;
}
QMatrix PostProcess::qtMatrix() const
{
    return ::autopanorama::qtMatrix(rotation());
}

cv::Mat PostProcess::cvMatrix(cv::InputArray image_to_rotate) const
{
    return ::autopanorama::cvMatrix(rotation(), image_to_rotate);
}

void PostProcess::updateRotated()
{
    QString text;
    text.sprintf("Angle: %.2f°", rotation());
    ui_->label_angle->setText(text);
    updater_thread_->setAngle(rotation());
}

void PostProcess::onSave()
{
    qDebug() << "Reading original image";
    cv::UMat original;
    cv::imread(output_path_.toStdString(), cv::IMREAD_UNCHANGED).copyTo(original);

    qDebug() << "Rotating the image";
    QPixmap qt_rotated = original_.transformed(qtMatrix());
    cv::Size size(qt_rotated.width(), qt_rotated.height());

    cv::UMat rotated;
    cv::warpAffine(original, rotated, cvMatrix(original), size, cv::INTER_CUBIC);

    if (ui_->radio_rotated->isChecked()) {
        QString path = getPostProcessPath();

        qDebug() << "Writing post-process to " << path;
        cv::imwrite(path.toStdString(), rotated);
    }

    if (ui_->radio_cut->isChecked()) {
        cv::UMat rotated_mask;
        std::vector<cv::UMat> channels;
        cv::split(original, channels);
        cv::UMat mask = channels.back();
        cv::warpAffine(mask, rotated_mask, cvMatrix(mask), size, cv::INTER_NEAREST);

        qDebug() << "Computing ROI";
        InnerCutFinder finder(rotated_mask);
        cv::Rect roi = finder.getROI();

        QString path = getPostProcessPath();

        qDebug() << "Writing post-process to " << path;
        cv::imwrite(path.toStdString(), rotated(roi));
    }
}

QString PostProcess::getPostProcessPath() const
{
    QFileInfo output(output_path_);
    QString basename = output.completeBaseName();
    QString suffix = output.suffix();
    QDir dir = output.absoluteDir();
    return generateNewFilename(basename + "_post_process." + suffix, dir);
}

} // autopanorama
