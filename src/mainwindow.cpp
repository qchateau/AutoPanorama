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
#include <QString>
#include <QCloseEvent>

#include <opencv2/core/ocl.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    worker_index(0)
{
    ui->setupUi(this);

    onBlenderTypeChange();
    onExposureCompensatorChange();
    updateInfos();
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    int nb = getNbQueued();
    if (nb == 0)
        close();
    else
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Quit ?",
                                      QString("All panoramas are not done yet. \nAre you sure you want to quit ?"),
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
            close();
        else
            event->ignore();
    }

}

void MainWindow::onMakePanoramaClicked() {
    QStringList files;
    bool clear_files = false;
    if (ui->selectedOnly_checkbox->isChecked())
    {
        files = ui->filesListWidget->getSelectedFilesList();
    }
    else
    {
        files = ui->filesListWidget->getFilesList();
        clear_files = true;
    }

    if (files.size() < 2) {
        QMessageBox::warning(this, "Not enough files", "Please select at least 2 files");
    } else {
        PanoramaMaker *worker = new PanoramaMaker;
        worker->setImages(files, ui->output_filename_lineedit->text(), ui->extension_combobox->currentText());
        configureWorker(worker);
        createWorkerUi(worker);
        connect(worker, SIGNAL(finished()), this, SLOT(runWorkers()));

        qDebug() << "Queuing worker";
        workers << worker;
        runWorkers();

        if (clear_files)
            ui->filesListWidget->clear();
    }
    updateStatusBar();
}

void MainWindow::configureWorker(PanoramaMaker *worker) {
    // Registration resolution
    worker->getStitcher()->setRegistrationResol(ui->regres_spinbox->value());

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
        worker->getStitcher()->setWaveCorrection(true);
        worker->getStitcher()->setWaveCorrectKind(detail::WAVE_CORRECT_HORIZ);
    } else if (wave_cor_kind_txt == QString("Vertical")) {
        worker->getStitcher()->setWaveCorrection(true);
        worker->getStitcher()->setWaveCorrectKind(detail::WAVE_CORRECT_VERT);
    } else {
        worker->getStitcher()->setWaveCorrection(false);
    }

    // Bundle adjuster
    worker->setBundleAdjusterMode(ui->bundleadj_combobox->currentText());

    // Panorama confidence
    worker->getStitcher()->setPanoConfidenceThresh(ui->confth_spinbox->value());

    // Exposure compensator mode
    worker->setExposureCompensatorMode(ui->expcomp_combobox->currentText(), ui->blocksize_spinbox->value());

    // Seam estimation resolution
    worker->getStitcher()->setSeamEstimationResol(ui->seamfinderres_spinbox->value());

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
    worker->getStitcher()->setCompositingResol(compositing_res);
}

void MainWindow::runWorkers() {
    if (workers.size() < (worker_index+1))
        return;
    PanoramaMaker *worker = workers[worker_index];
    if (worker->isFinished()) {
        qDebug() << "Finished worker";
        ++worker_index;
        runWorkers();
    } else if (!worker->isRunning()) {
        qDebug() << "Starting worker";
        worker->start();
    }
}

void MainWindow::createWorkerUi(PanoramaMaker *worker) {
    QProgressBar *progress_bar = new QProgressBar;
    progress_bar->setRange(0,100);
    progress_bar->setFormat(worker->getOutputFilename()+" : %p%");
    progress_bar->setAlignment(Qt::AlignCenter);
    progress_bar->setValue(0);
    progress_bar->setToolTip(worker->getStitcherConfString());
    progress_bars << progress_bar;
    connect(worker, SIGNAL(percentage(int)), progress_bar, SLOT(setValue(int)));
    connect(worker, SIGNAL(failed(QString)), this, SLOT(onWorkerFailed(QString)));
    connect(worker, SIGNAL(done()), this, SLOT(onWorkerDone()));
    connect(worker, SIGNAL(percentage(int)), this, SLOT(updateStatusBar()));
    connect(worker, SIGNAL(failed(QString)), this, SLOT(updateStatusBar()));
    connect(worker, SIGNAL(done()), this, SLOT(updateStatusBar()));
    ui->tabProgressLayout->addWidget(progress_bar);
}

void MainWindow::onWorkerFailed(QString msg)
{
    PanoramaMaker* sender = qobject_cast<PanoramaMaker*>(QObject::sender());
    int idx = getWorkerIndex(sender);
    if (idx < 0 || idx > (progress_bars.size() - 1))
        return;
    QProgressBar *pb = progress_bars[idx];
    pb->setFormat(QString("%1 : Failed (%2)").arg(sender->getOutputFilename()).arg(msg));
    pb->setValue(100);
    pb->setStyleSheet("QProgressBar::chunk{background-color:red}");
}

void MainWindow::onWorkerDone()
{
    PanoramaMaker* sender = qobject_cast<PanoramaMaker*>(QObject::sender());
    int idx = getWorkerIndex(sender);
    if (idx < 0 || idx > (progress_bars.size() - 1))
        return;
    QProgressBar *pb = progress_bars[idx];
    pb->setStyleSheet("QProgressBar::chunk{background-color:green}");
    pb->setToolTip(sender->getStitcherConfString());
    pb->setFormat(QString("%1 : Done !").arg(sender->getOutputFilename()));
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
    } else {
        ui->sharpness_label->hide();
        ui->sharpness_spinbox->hide();
        ui->nbands_label->hide();
        ui->nbands_spinbox->hide();
    }
}

void MainWindow::onExposureCompensatorChange() {
    QString type = ui->expcomp_combobox->currentText();
    if (type == QString("Blocks Gain") ||
        type == QString("Blocks BGR"))
    {
        ui->blocksize_label->show();
        ui->blocksize_spinbox->show();
    }
    else
    {
        ui->blocksize_label->hide();
        ui->blocksize_spinbox->hide();
    }
}

void MainWindow::updateInfos()
{
    QString yes("Yes"), no("No");
    ui->haveopencl_value->setText(cv::ocl::haveOpenCL() ? yes : no);
}

int MainWindow::getNbQueued()
{
    int nb = 0;
    for (int i = 0; i < workers.size(); ++i)
    {
        if (!workers[i]) continue;
        PanoramaMaker::Status status = workers[i]->getStatus();
        if (status == PanoramaMaker::STOPPED || status == PanoramaMaker::WORKING)
                ++nb;
    }
    return nb;
}

int MainWindow::getNbDone()
{
    int nb = 0;
    for (int i = 0; i < workers.size(); ++i)
    {
        if (!workers[i]) continue;
        PanoramaMaker::Status status = workers[i]->getStatus();
        if (status == PanoramaMaker::DONE)
                ++nb;
    }
    return nb;
}

int MainWindow::getNbFailed()
{
    int nb = 0;
    for (int i = 0; i < workers.size(); ++i)
    {
        if (!workers[i]) continue;
        PanoramaMaker::Status status = workers[i]->getStatus();
        if (status == PanoramaMaker::FAILED)
                ++nb;
    }
    return nb;
}

int MainWindow::getCurrentProgress()
{
    if (workers.size() < (worker_index+1) || !workers[worker_index])
        return 0;

    if (workers[worker_index]->getStatus() == PanoramaMaker::WORKING)
    {
        return workers[worker_index]->getProgress();
    }
    return 0;
}

void MainWindow::updateStatusBar()
{
    QString text = QString("Current job : %1%       Jobs left : %2      Jobs done : %3      Jobs failed : %4")
            .arg(getCurrentProgress()).arg(getNbQueued()).arg(getNbDone()).arg(getNbFailed());
    ui->statusBar->showMessage(text);
}

int MainWindow::getWorkerIndex(PanoramaMaker *worker)
{
    for (int i = 0; i < workers.size(); ++i)
    {
        if (workers[i] == worker)
            return i;
    }
    return -1;
}
