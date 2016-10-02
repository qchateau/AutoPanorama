#ifndef PANORAMAMAKER_H
#define PANORAMAMAKER_H

#include <QThread>

class PanoramaMaker : public QThread
{
    Q_OBJECT
public:
    explicit PanoramaMaker(QObject *parent = 0);
    void setImages(QStringList files);
    void run();
    QString out_filename() { return output_filename; }

private:
    QStringList images_path;
    QString output_filename;

signals:
    void percentage(int);
    void done();

public slots:
};

#endif // PANORAMAMAKER_H
