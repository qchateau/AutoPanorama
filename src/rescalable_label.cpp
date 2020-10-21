#include "rescalable_label.h"

namespace autopanorama {

RescalableLabel::RescalableLabel(QWidget* parent) : QLabel(parent)
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

void RescalableLabel::mousePressEvent(QMouseEvent* event)
{
    QLabel::mousePressEvent(event);
    clicked();
}

void RescalableLabel::updatePixmap()
{
    QPixmap scaled = original_.scaled(
        this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QLabel::setPixmap(scaled);
}

} // autopanorama
