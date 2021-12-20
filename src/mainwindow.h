#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "panoramamaker.h"
#include "ui_mainwindow.h"

#include <map>
#include <memory>
#include <opencv2/stitching.hpp>

#include <QHBoxLayout>
#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>

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
    int getNbTotal() { return workers_.size(); }
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
    void setupLogsTable();
    void openPostProcess(const QString& output_path);
    void startWorker(PanoramaMaker& worker);
    void createWorkerUi(std::shared_ptr<PanoramaMaker> worker);
    void configureWorker(PanoramaMaker& worker);
    QString oclDeviceTypeToString(int type);

    struct ProgressBarContent {
        bool auto_open_post_process;
        QProgressBar* pb;
        QPushButton* close;
        QPushButton* post_process;
        std::shared_ptr<QHBoxLayout> layout;
        std::shared_ptr<PanoramaMaker> worker;
    };

    std::unique_ptr<Ui::MainWindow> ui_;

    QList<std::shared_ptr<PanoramaMaker>> workers_;
    std::map<QObject*, ProgressBarContent> progress_bars_;
    int worker_index_;

    cv::Stitcher stitcher_;
    bool manual_output_filename_;
    bool manual_output_dir_;
    int max_filename_length_;

private slots:
    void closeSenderWorker();
};

} // autopanorama

#endif // MAINWINDOW_H
