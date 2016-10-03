#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressBar>
#include <QLineEdit>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    fs_model = new QFileSystemModel;

    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.bmp";
    fs_model->setNameFilters(filters);
    fs_model->setNameFilterDisables(false);

    QString root_dir = "";
    fs_model->setRootPath(root_dir);

    ui->setupUi(this);
    ui->fsView->setModel(fs_model);
    ui->fsView->setRootIndex(fs_model->index(root_dir));
    ui->fsView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete fs_model;
}

QStringList MainWindow::getSelectedFiles() {
    QStringList files;
    QModelIndexList idx = ui->fsView->selectionModel()->selectedRows();
    for(int i=0; i<idx.size(); i++) {
        QString path = fs_model->filePath(idx[i]);
        QFileInfo check_file(path);
        if (check_file.exists() && check_file.isFile()) {
            files << path;
        }
    }
    return files;
}

void MainWindow::onSelectDirClicked() {
    QString source_dir = QFileDialog::getExistingDirectory();
    qDebug() << "Selected directory : " << source_dir;

    fs_model->setRootPath(source_dir);
    ui->fsView->setRootIndex(fs_model->index(source_dir));
}

void MainWindow::onMakePanoramaClicked() {
    QStringList files = getSelectedFiles();
    qDebug() << "Making panorama from : " << files;

    if (files.size() < 2) {
        QMessageBox::warning(this, "Not enough files", "Please select at least 2 files");
    } else {
        PanoramaMaker *worker = new PanoramaMaker;
        int nr = 0;
        QFileInfo output_fileinfo;
        do {
            QString output_filename(ui->output_filename_lineedit->text());
            if (nr++ > 0) {
                output_filename += "_"+QString::number(nr);
            }
            output_filename += ui->extension_combobox->currentText();
            output_fileinfo = QFileInfo(QDir(QFileInfo(files[0]).absoluteDir()).filePath(output_filename));
        } while (output_fileinfo.exists());

        QFile fil(output_fileinfo.absoluteFilePath());
        fil.open(QIODevice::ReadWrite);

        worker->setImages(files, output_fileinfo.absoluteFilePath());
        worker->setWarpMode(ui->warpmode_combobox->currentText());
        worker->setDownscale(ui->downscale_spinbox->value()/100.0);
        connect(worker, SIGNAL(finished()), this, SLOT(runWorkers()));
        createWorkerUi(worker);
        workers << worker;
        runWorkers();
    }
}

void MainWindow::runWorkers() {
    if (workers.isEmpty())
        return;
    PanoramaMaker *worker = workers[0];
    if (worker->isFinished()) {
        qDebug() << "Deleting worker";
        workers.pop_front();
        runWorkers();
    } else if (!worker->isRunning()) {
        qDebug() << "Starting worker";
        worker->start();
    }
}

void MainWindow::createWorkerUi(PanoramaMaker *worker) {
    QProgressBar *progress_bar = new QProgressBar;
    progress_bar->setRange(0,100);
    progress_bar->setFormat(worker->out_fileinfo().fileName()+" : %p%");
    progress_bar->setAlignment(Qt::AlignCenter);
    progress_bar->setValue(0);
    connect(worker, SIGNAL(percentage(int)), progress_bar, SLOT(setValue(int)));
    //connect(worker, SIGNAL(finished()), progress_bar, SLOT(close()));
    ui->tabProgressLayout->addWidget(progress_bar);
}
