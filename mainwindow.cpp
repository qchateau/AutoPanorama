#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressBar>
#include <QLineEdit>
#include <QFile>
#include <QToolTip>
#include <QPoint>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    fs_model = new QFileSystemModel;

    QStringList filters;
    QString root_dir = QDir::homePath();
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp";
    filters << "*.PNG" << "*.JPG" << "*.JPEG" << "*.BMP";
    fs_model->setNameFilters(filters);
    fs_model->setNameFilterDisables(false);
    fs_model->setRootPath(root_dir);

    ui->setupUi(this);
    ui->fsView->setModel(fs_model);
    ui->fsView->setRootIndex(fs_model->index(root_dir));
    ui->fsView->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    onBlenderTypeChange();
    onExposureCompensatorChange();
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
    QString source_dir = QFileDialog::getExistingDirectory(this, "Open directory", QDir::homePath(), 0);
    qDebug() << "Selected directory : " << source_dir;

    fs_model->setRootPath(source_dir);
    ui->fsView->setRootIndex(fs_model->index(source_dir));
    ui->fsView->clearSelection();
}

void MainWindow::onMakePanoramaClicked() {
    QStringList files = getSelectedFiles();
    qDebug() << "Making panorama from : " << files;

    if (files.size() < 2) {
        QMessageBox::warning(this, "Not enough files", "Please select at least 2 files");
    } else {
        PanoramaMaker *worker = new PanoramaMaker;
        worker->setImages(files, ui->output_filename_lineedit->text(), ui->extension_combobox->currentText());
        configureWorker(worker);
        createWorkerUi(worker);
        connect(worker, SIGNAL(finished()), this, SLOT(runWorkers()));
        workers << worker;
        runWorkers();
    }
}

void MainWindow::configureWorker(PanoramaMaker *worker) {
    // Registration resolution
    worker->get_stitcher()->setRegistrationResol(ui->regres_spinbox->value());

    // Feature finder mode
    worker->setFeaturesFinderMode(ui->featuresfinder_combobox->currentText());

    // Feature matching mode and confidence
    worker->setFeaturesMatchingMode(ui->featuresmatcher_combobox->currentText(),
                                    ui->featuresmatcherconf_spinbox->value());

    // Warp mode
    worker->setWarpMode(ui->warpmode_combobox->currentText());

    // Wave correction
    QString wave_cor_kind_txt = ui->wavecorkind_combobox->currentText();
    if (wave_cor_kind_txt == QString("Horizontal")) {
        worker->get_stitcher()->setWaveCorrection(true);
        worker->get_stitcher()->setWaveCorrectKind(detail::WAVE_CORRECT_HORIZ);
    } else if (wave_cor_kind_txt == QString("Vertical")) {
        worker->get_stitcher()->setWaveCorrection(true);
        worker->get_stitcher()->setWaveCorrectKind(detail::WAVE_CORRECT_VERT);
    } else {
        worker->get_stitcher()->setWaveCorrection(false);
    }

    // Bundle adjuster
    worker->setBundleAdjusterMode(ui->bundleadj_combobox->currentText());

    // Panorama confidence
    worker->get_stitcher()->setPanoConfidenceThresh(ui->confth_spinbox->value());

    // Exposure compensator mode
    worker->setExposureCompensatorMode(ui->expcomp_combobox->currentText(), ui->blocksize_spinbox->value());

    // Seam estimation resolution
    worker->get_stitcher()->setSeamEstimationResol(ui->seamfinderres_spinbox->value());

    // Seam finder mode
    worker->setSeamFinderMode(ui->seamfindermode_combobox->currentText());

    // Blender
    QString blender_type = ui->blendertype_combobox->currentText();
    double blender_param = -1;
    if (blender_type == QString("Feather")) {
        blender_param = ui->sharpness_spinbox->value();
    } else if (blender_type == QString("Multiband")) {
        blender_param = ui->nbands_spinbox->value();
    }
    worker->setBlenderMode(blender_type, blender_param);

    // Compositing resolution
    double compositing_res = ui->compositingres_spinbox->value();
    if (compositing_res <= 0) {
        compositing_res = Stitcher::ORIG_RESOL;
    }
    worker->get_stitcher()->setCompositingResol(compositing_res);
}

void MainWindow::runWorkers() {
    if (workers.isEmpty())
        return;
    PanoramaMaker *worker = workers[0];
    if (worker->isFinished()) {
        qDebug() << "Deleting worker";
        workers.pop_front();
        delete worker;
        runWorkers();
    } else if (!worker->isRunning()) {
        qDebug() << "Starting worker";
        worker->start();
    }
}

void MainWindow::createWorkerUi(PanoramaMaker *worker) {
    QProgressBar *progress_bar = new QProgressBar;
    progress_bar->setRange(0,100);
    progress_bar->setFormat(worker->get_output_filename()+" : %p%");
    progress_bar->setAlignment(Qt::AlignCenter);
    progress_bar->setValue(0);
    progress_bar->setToolTip(worker->getStitcherConfString());
    connect(worker, SIGNAL(percentage(int)), progress_bar, SLOT(setValue(int)));
    connect(worker, SIGNAL(failed(QString)), progress_bar, SLOT(close()));
    //connect(worker, SIGNAL(finished()), progress_bar, SLOT(close()));
    ui->tabProgressLayout->addWidget(progress_bar);
}

void MainWindow::onBlenderTypeChange() {
    QString type = ui->blendertype_combobox->currentText();
    if (type == QString("Multiband")) {
        ui->sharpness_label->hide();
        ui->sharpness_spinbox->hide();
        ui->nbands_label->show();
        ui->nbands_spinbox->show();
    } else if (type == QString("Feather")) {
        ui->sharpness_label->show();
        ui->sharpness_spinbox->show();
        ui->nbands_label->hide();
        ui->nbands_spinbox->hide();
    }
}

void MainWindow::onExposureCompensatorChange() {
    QString type = ui->expcomp_combobox->currentText();
    if (type == QString("Blocks Gain")) {
        ui->blocksize_label->show();
        ui->blocksize_spinbox->show();
    } else {
        ui->blocksize_label->show();
        ui->blocksize_spinbox->show();
    }
}

