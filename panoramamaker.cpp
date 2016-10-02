#include "panoramamaker.h"

#include <QtDebug>
#include <QFileInfo>
#include <unistd.h>

PanoramaMaker::PanoramaMaker(QObject *parent) : QThread(parent)
{}

void PanoramaMaker::run() {
    for (int i=0; i<=10; i++) {
        emit percentage(i*10);
        sleep(1);
    }

    emit done();
}

void PanoramaMaker::setImages(QStringList files) {
    images_path = files;
    output_filename = QString();
    for (int i=0; i<files.size(); ++i) {
        QFileInfo file(files[i]);
        output_filename += file.baseName();
        if (i < files.size()-1) {
            output_filename += "_";
        }
    }
}
