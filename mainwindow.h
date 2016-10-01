#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "appbehaviour.h"

#include <QMainWindow>

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

public slots:
    void onSelectDirClicked();
    void onMakePanoramaClicked();
};

#endif // MAINWINDOW_H
