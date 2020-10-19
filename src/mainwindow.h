#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "panoramamaker.h"

#include <map>
#include <opencv2/stitching.hpp>

#include <QHBoxLayout>
#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>

namespace Ui {
class MainWindow;
}

namespace autopanorama {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
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
    void onWorkerFailed(QString msg = QString());
    void onWorkerDone();
    void onBlenderTypeChange();
    void onExposureCompensatorChange();
    void onOutputFilenameChanged(QString edit = QString());
    void onOutputDirChanged(QString edit = QString());
    void onOutputFilenameEdit(QString edit = QString());
    void onOutputDirEdit(QString edit = QString());
    void onSelectOutputDirClicked();
    void onFastSettingsChanged();
    void resetAlgoSetting();
    void updateMakeEnabled();
    void updateOCL();
    void updateEigen();
    void updateIPP();
    void updateArch();
    void updateVersion();
    void updateOutputDirFilename();
    void updateStatusBar();

protected:
    void closeEvent(QCloseEvent* event);

private:
    void startWorker(PanoramaMaker* worker);
    void createWorkerUi(PanoramaMaker* worker);
    void configureWorker(PanoramaMaker* worker);
    QString oclDeviceTypeToString(int type);

    struct ProgressBarContent {
        QProgressBar* pb;
        QPushButton* close;
        PanoramaMaker* worker;
        QHBoxLayout* layout;
    };

    Ui::MainWindow* ui;

    QList<PanoramaMaker*> workers;
    std::map<QObject*, ProgressBarContent> progress_bars;
    int worker_index;

    cv::Stitcher stitcher;
    bool manual_output_filename, manual_output_dir;
    int max_filename_length;

private slots:
    void closeSenderWorker();
};

} // autopanorama

#endif // MAINWINDOW_H
