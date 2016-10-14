#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "panoramamaker.h"

#include <opencv2/stitching.hpp>

#include <QMainWindow>
#include <QTimer>
#include <QProgressBar>

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

public slots:
    void onMakePanoramaClicked();
    void runWorkers();
    void onWorkerFailed(QString msg=QString());
    void onWorkerDone();
    void onBlenderTypeChange();
    void onExposureCompensatorChange();
    void onOutputFilenameChanged(QString edit=QString());
    void onOutputDirChanged(QString edit=QString());
    void onOutputFilenameEdit(QString edit=QString());
    void onOutputDirEdit(QString edit=QString());
    void onSelectOutputDirClicked();
    void updateMakeEnabled();
    void updateInfos();
    void updateOutputDirFilename();
    void updateStatusBar();

protected:
    void closeEvent(QCloseEvent *event);

private:
    void createWorkerUi(PanoramaMaker *worker);
    void configureWorker(PanoramaMaker *worker);
    int getWorkerIndex(PanoramaMaker* worker);

    Ui::MainWindow *ui;

    QList<PanoramaMaker*> workers;
    QList<QProgressBar*> progress_bars;
    int worker_index;

    Stitcher stitcher;
    bool manual_output_filename, manual_output_dir;
    int max_filename_length;
};

#endif // MAINWINDOW_H
