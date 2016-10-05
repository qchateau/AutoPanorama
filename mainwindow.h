#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "panoramamaker.h"

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

private:
    Ui::MainWindow *ui;
    QFileSystemModel *fs_model;
    QStringList getSelectedFiles();
    void createWorkerUi(PanoramaMaker *worker);
    void configureWorker(PanoramaMaker *worker);

    QList<PanoramaMaker*> workers;

public slots:
    void onSelectDirClicked();
    void onMakePanoramaClicked();
    void runWorkers();
    void onBlenderTypeChange();
    void onExposureCompensatorChange();
};

#endif // MAINWINDOW_H
