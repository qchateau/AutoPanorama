#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "panoramamaker.h"

#include <opencv2/stitching.hpp>

#include <QMainWindow>
#include <QFileSystemModel>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int getNbQueued();
    int getNbDone();
    int getNbFailed();
    int getNbFinished() { return getNbDone() + getNbFailed(); }
    int getNbTotal() { return workers.size(); }
    int getCurrentProgress();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    void createWorkerUi(PanoramaMaker *worker);
    void configureWorker(PanoramaMaker *worker);
    int getWorkerIndex(PanoramaMaker* worker);

    QList<PanoramaMaker*> workers;
    QList<QProgressBar*> progress_bars;
    int worker_index;

    Stitcher stitcher;

public slots:
    void onMakePanoramaClicked();
    void runWorkers();
    void onBlenderTypeChange();
    void onExposureCompensatorChange();
    void onWorkerFailed(QString msg);
    void onWorkerDone();
    void updateInfos();
    void updateStatusBar();
};

#endif // MAINWINDOW_H
