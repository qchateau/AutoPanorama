#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "panoramamaker.h"

#include <opencv2/stitching.hpp>
#include <map>

#include <QMainWindow>
#include <QTimer>
#include <QProgressBar>
#include <QPushButton>
#include <QHBoxLayout>

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
    void updateOCL();
    void updateEigen();
    void updateIPP();
    void updateArch();
    void updateOutputDirFilename();
    void updateStatusBar();

protected:
    void closeEvent(QCloseEvent *event);

private:
    void startWorker(PanoramaMaker *worker);
    void createWorkerUi(PanoramaMaker *worker);
    void configureWorker(PanoramaMaker *worker);
    QString oclDeviceTypeToString(int type);

    struct ProgressBarContent {
        QProgressBar *pb;
        QPushButton *close;
        PanoramaMaker *worker;
        QHBoxLayout *layout;
    };

    Ui::MainWindow *ui;

    QList<PanoramaMaker*> workers;
    std::map<QObject*, ProgressBarContent> progress_bars;
    int worker_index;

    Stitcher stitcher;
    bool manual_output_filename, manual_output_dir;
    int max_filename_length;

private slots:
    void closeSenderWorker();
};

#endif // MAINWINDOW_H
