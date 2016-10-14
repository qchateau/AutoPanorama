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
    void updateInfos();
    void updateOutputDirFilename();
    void updateStatusBar();

protected:
    void closeEvent(QCloseEvent *event);

private:
    void createWorkerUi(PanoramaMaker *worker);
    void configureWorker(PanoramaMaker *worker);

    struct ProgressBarContent {
        QProgressBar *pb;
        QPushButton *hide, *close;
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
    void hideSenderWorker();
    void closeSenderWorker();
};

#endif // MAINWINDOW_H
