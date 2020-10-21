#ifndef RESCALABLE_LABEL_H
#define RESCALABLE_LABEL_H

#include <QLabel>
#include <QPixmap>

#include <opencv2/opencv.hpp>

namespace autopanorama {

class RescalableLabel : public QLabel {
    Q_OBJECT
signals:
    void clicked();

public:
    RescalableLabel(QWidget* parent = nullptr);
    void setPixmap(QPixmap pixmap);

protected:
    void resizeEvent(QResizeEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void updatePixmap();

    QPixmap original_;
};

} // autopanorama

#endif // RESCALABLE_LABEL_H
