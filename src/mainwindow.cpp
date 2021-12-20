#include "mainwindow.h"
#include "env.h"
#include "post_process.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QString>
#include <QToolTip>
#include <QtDebug>

#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>

namespace autopanorama {
namespace {

class LogSink {
public:
    static void onMessageOutput(
        QtMsgType type,
        const QMessageLogContext&,
        const QString& msg)
    {
        QString level;
        switch (type) {
        case QtDebugMsg:
            level = "DEBUG";
            break;
        case QtInfoMsg:
            level = "INFO";
            break;
        case QtWarningMsg:
            level = "WARNING";
            break;
        case QtCriticalMsg:
            level = "ERROR";
            break;
        case QtFatalMsg:
            level = "FATAL";
            break;
        }

        auto line = level + QString(7 - level.size(), ' ') + ": " + msg;
        std::cerr << line.toStdString() << std::endl;

        QString newText;
        {
            std::unique_lock<std::mutex> lock(mtx_);
            static QStringList lines;
            lines.append(line);
            if (lines.size() > 100000) {
                lines.pop_front();
            }
            if (browser_) {
                QMetaObject::invokeMethod(
                    browser_, "setPlainText", Q_ARG(QString, lines.join('\n')));
            }
        }
    }

    static void setTextBrowser(QTextBrowser* browser)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        browser_ = browser;
    }

private:
    static std::mutex mtx_;
    static QTextBrowser* browser_;
};

std::mutex LogSink::mtx_;
QTextBrowser* LogSink::browser_ = nullptr;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui_(std::make_unique<Ui::MainWindow>()),
      manual_output_filename_(false),
      manual_output_dir_(false),
      max_filename_length_(50)
{
    qRegisterMetaType<QVector<int>>();
    ui_->setupUi(this);
    setupLogsTable();

    qInfo() << cv::getBuildInformation().c_str();

    updateOutputDirFilename();
    onFastSettingsChanged();
    onBlenderTypeChange();
    onExposureCompensatorChange();
    updateOCL();
    updateEigen();
    updateIPP();
    updateArch();
    updateVersion();
    updateStatusBar();
    updateMakeEnabled();

    for (const QString& ext : PanoramaMaker::getSupportedImageExtensions()) {
        ui_->filesListWidget->addSupportedExtension(ext);
    }
    for (const QString& ext : PanoramaMaker::getSupportedVideoExtensions()) {
        ui_->filesListWidget->addSupportedExtension(ext);
    }
}

MainWindow::~MainWindow()
{
    LogSink::setTextBrowser(nullptr);
    qInstallMessageHandler(0);
}

int MainWindow::getNbQueued()
{
    int nb = 0;
    for (int i = 0; i < workers_.size(); ++i) {
        if (!workers_[i])
            continue;
        PanoramaMaker::Status status = workers_[i]->getStatus();
        if (status == PanoramaMaker::STOPPED || status == PanoramaMaker::WORKING)
            ++nb;
    }
    return nb;
}

int MainWindow::getNbDone()
{
    int nb = 0;
    for (int i = 0; i < workers_.size(); ++i) {
        if (!workers_[i])
            continue;
        PanoramaMaker::Status status = workers_[i]->getStatus();
        if (status == PanoramaMaker::DONE)
            ++nb;
    }
    return nb;
}

int MainWindow::getNbFailed()
{
    int nb = 0;
    for (int i = 0; i < workers_.size(); ++i) {
        if (!workers_[i])
            continue;
        PanoramaMaker::Status status = workers_[i]->getStatus();
        if (status == PanoramaMaker::FAILED)
            ++nb;
    }
    return nb;
}

int MainWindow::getCurrentProgress()
{
    for (int i = 0; i < workers_.size(); ++i) {
        if (workers_[i]->getStatus() == PanoramaMaker::WORKING)
            return workers_[i]->getProgress();
    }
    return 0;
}

void MainWindow::onMakePanoramaClicked()
{
    QStringList files;
    bool clear_files = false;
    if (ui_->selectedOnly_checkbox->isChecked()) {
        files = ui_->filesListWidget->getSelectedFilesList();
    }
    else {
        files = ui_->filesListWidget->getFilesList();
        clear_files = true;
    }

    QStringList supported_image_ext = PanoramaMaker::getSupportedImageExtensions();
    QStringList supported_video_ext = PanoramaMaker::getSupportedVideoExtensions();
    QStringList images, videos;

    for (const QString& file : files) {
        QString ext = QFileInfo(file).suffix().toLower();
        if (supported_image_ext.contains(ext)) {
            images.append(file);
        }
        else if (supported_video_ext.contains(ext)) {
            videos.append(file);
        }
        else {
            QMessageBox::warning(
                this, "Invalid files", "File " + file + " is not supported.");
            return;
        }
    }

    if (images.size() < 2 && videos.size() < 1) {
        QMessageBox::warning(
            this,
            "Not enough files",
            "Please select at least 2 images or 1 video");
        return;
    }

    std::shared_ptr<PanoramaMaker> worker = std::make_shared<PanoramaMaker>();

    if (images.size() > 0) {
        try {
            worker->setImages(images);
        }
        catch (const std::invalid_argument& e) {
            QMessageBox::warning(this, "Invalid files", e.what());
            return;
        }
    }
    if (videos.size() > 0) {
        try {
            worker->setVideos(videos);
        }
        catch (const std::invalid_argument& e) {
            QMessageBox::warning(this, "Invalid files", e.what());
            return;
        }
    }

    worker->setOutput(
        ui_->output_filename_lineedit->text(),
        ui_->output_dir_lineedit->text(),
        false);
    configureWorker(*worker);
    createWorkerUi(worker);
    connect(worker.get(), &PanoramaMaker::finished, this, &MainWindow::runWorkers);

    qDebug() << "Queuing worker";
    workers_ << worker;
    runWorkers();

    if (clear_files)
        ui_->filesListWidget->clear();
    updateStatusBar();
}

void MainWindow::runWorkers()
{
    for (int i = 0; i < workers_.size(); ++i) {
        if (workers_[i]->isRunning())
            break;
        if (workers_[i]->getStatus() == PanoramaMaker::STOPPED) {
            startWorker(*workers_[i]);
            break;
        }
    }
}

void MainWindow::onWorkerFailed(QString msg)
{
    PanoramaMaker* sender = qobject_cast<PanoramaMaker*>(QObject::sender());
    QProgressBar* pb = progress_bars_[sender].pb;
    if (!pb)
        return;
    pb->setFormat(
        QString("%1 : Failed (%2)").arg(sender->getOutputFilename()).arg(msg));
    pb->setValue(100);
    pb->setStyleSheet("QProgressBar::chunk{background-color:red}");
    pb->setToolTip(pb->toolTip() + "\n\nError:\n" + msg);
    progress_bars_[sender].close->setText("Hide");
    progress_bars_[sender].close->setEnabled(true);
}

void MainWindow::onWorkerDone()
{
    PanoramaMaker* sender = qobject_cast<PanoramaMaker*>(QObject::sender());
    QProgressBar* pb = progress_bars_[sender].pb;
    if (!pb)
        return;
    pb->setStyleSheet("QProgressBar::chunk{background-color:green}");
    pb->setFormat(QString("%1 : Done ! (%2s total, %3s processing)")
                      .arg(sender->getOutputFilename())
                      .arg(QString::number(sender->getTotalTime(), 'f', 1))
                      .arg(QString::number(sender->getProcTime(), 'f', 1)));
    progress_bars_[sender].close->setText("Hide");
    progress_bars_[sender].close->setEnabled(true);
    progress_bars_[sender].post_process->setEnabled(true);
    QString output_path = progress_bars_[sender].worker->getOutputFilePath();

    if (progress_bars_[sender].auto_open_post_process) {
        openPostProcess(output_path);
    }

    connect(progress_bars_[sender].post_process, &QPushButton::clicked, this, [=]() {
        openPostProcess(output_path);
    });
}

void MainWindow::onBlenderTypeChange()
{
    QString type = ui_->blendertype_combobox->currentText();
    if (type == QString("Multiband")) {
        ui_->sharpness_label->hide();
        ui_->sharpness_spinbox->hide();
        ui_->nbands_label->show();
        ui_->nbands_spinbox->show();
    }
    else if (type == QString("Feather")) {
        ui_->sharpness_label->show();
        ui_->sharpness_spinbox->show();
        ui_->nbands_label->hide();
        ui_->nbands_spinbox->hide();
    }
    else {
        ui_->sharpness_label->hide();
        ui_->sharpness_spinbox->hide();
        ui_->nbands_label->hide();
        ui_->nbands_spinbox->hide();
    }
}

void MainWindow::onExposureCompensatorChange()
{
    QString mode = ui_->expcomp_mode_combobox->currentText();
    if (mode == QString("Blocks") || mode == QString("Combined")) {
        ui_->blocksize_label->show();
        ui_->blocksize_spinbox->show();
        ui_->exp_sim_th_spinbox->setValue(0.30);
    }
    else {
        ui_->blocksize_label->hide();
        ui_->blocksize_spinbox->hide();
        ui_->exp_sim_th_spinbox->setValue(1);
    }
    if (mode == QString("None")) {
        ui_->nfeed_label->hide();
        ui_->nfeed_spinbox->hide();
        ui_->expcomp_type_combobox->hide();
        ui_->expcomp_type_label->hide();
        ui_->exp_sim_th_label->hide();
        ui_->exp_sim_th_spinbox->hide();
    }
    else {
        ui_->nfeed_label->show();
        ui_->nfeed_spinbox->show();
        ui_->expcomp_type_combobox->show();
        ui_->expcomp_type_label->show();
        ui_->exp_sim_th_label->show();
        ui_->exp_sim_th_spinbox->show();
    }
}

void MainWindow::onOutputFilenameChanged(QString /*edit*/)
{
    updateMakeEnabled();
}

void MainWindow::onOutputDirChanged(QString edit)
{
    QPalette palette;
    QString tooltip;

    if (edit.isEmpty() || !QDir(edit).exists()) {
        palette.setColor(QPalette::Text, Qt::red);
        tooltip = "This directory doesn't exists";
    }

    qobject_cast<QLineEdit*>(sender())->setPalette(palette);
    qobject_cast<QLineEdit*>(sender())->setToolTip(tooltip);
    updateMakeEnabled();
}

void MainWindow::onOutputFilenameEdit(QString edit)
{
    if (edit.isEmpty())
        manual_output_filename_ = false;
    else
        manual_output_filename_ = true;
}

void MainWindow::onOutputDirEdit(QString edit)
{
    if (edit.isEmpty())
        manual_output_dir_ = false;
    else
        manual_output_dir_ = true;
}

void MainWindow::onSelectOutputDirClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "Select output directory", ui_->output_dir_lineedit->text());
    if (!dir.isEmpty()) {
        ui_->output_dir_lineedit->setText(dir);
        manual_output_dir_ = true;
    }
    else {
        manual_output_dir_ = false;
        updateOutputDirFilename();
    }
}

void MainWindow::onFastSettingsChanged()
{
    resetAlgoSetting();
    int exp_comp = ui_->fast_excomp_value->value();
    switch (exp_comp) {
    case 0: // Very fast
        ui_->nfeed_spinbox->setValue(1);
        ui_->seamfinderres_spinbox->setValue(0.1);
        ui_->regres_spinbox->setValue(0.6);
        ui_->expcomp_mode_combobox->setCurrentText("Simple");
        break;
    case 1: // Fast
        ui_->nfeed_spinbox->setValue(1);
        ui_->seamfinderres_spinbox->setValue(0.1);
        ui_->regres_spinbox->setValue(0.6);
        ui_->expcomp_mode_combobox->setCurrentText("Combined");
        break;
    case 2: // Slow
        ui_->nfeed_spinbox->setValue(3);
        ui_->seamfinderres_spinbox->setValue(0.1);
        ui_->regres_spinbox->setValue(0.6);
        ui_->expcomp_mode_combobox->setCurrentText("Combined");
        break;
    case 3: // Very slow
        ui_->nfeed_spinbox->setValue(3);
        ui_->seamfinderres_spinbox->setValue(0.2);
        ui_->regres_spinbox->setValue(1.0);
        ui_->expcomp_mode_combobox->setCurrentText("Combined");
        break;
    }

    int pan_size = ui_->fast_pan_size_value->value();
    switch (pan_size) {
    case 0: // Small
        ui_->compositingres_spinbox->setValue(1);
        break;
    case 1: // Medium
        ui_->compositingres_spinbox->setValue(5);
        break;
    case 2: // Full size
        ui_->compositingres_spinbox->setValue(0);
        break;
    }

    QString proj_type = "Spherical";
    if (ui_->fast_proj_type_sph->isChecked())
        proj_type = "Spherical";
    else if (ui_->fast_proj_type_cyl->isChecked())
        proj_type = "Cylindrical";
    else if (ui_->fast_proj_type_pla->isChecked())
        proj_type = "Perspective";
    ui_->warpmode_combobox->setCurrentText(proj_type);
}

void MainWindow::resetAlgoSetting()
{
    ui_->post_process_checkbox->setCheckState(Qt::Checked);
    ui_->regres_spinbox->setValue(1.0);
    ui_->featuresfinder_combobox->setCurrentText("AKAZE");
    ui_->featuresmatcher_combobox->setCurrentText("Best of 2 nearest");
    ui_->featuresmatcherconf_spinbox->setValue(0.65);
    ui_->warpmode_combobox->setCurrentText("Spherical");
    ui_->wavecorkind_combobox->setCurrentText("Auto");
    ui_->bundleadj_combobox->setCurrentText("Ray");
    ui_->confth_spinbox->setValue(1.0);

    ui_->expcomp_mode_combobox->setCurrentText("Combined");
    ui_->expcomp_type_combobox->setCurrentText("BGR");
    ui_->nfeed_spinbox->setValue(3);
    ui_->blocksize_spinbox->setValue(32);
    ui_->exp_sim_th_spinbox->setValue(0.30);

    ui_->seamfinderres_spinbox->setValue(0.2);
    ui_->seamfindermode_combobox->setCurrentText("Graph cut color");

    ui_->blendertype_combobox->setCurrentText("Multiband");
    ui_->nbands_spinbox->setValue(3);

    ui_->compositingres_spinbox->setValue(0);
    ui_->interp_combobox->setCurrentText("Cubic");

    ui_->images_per_videos_spinbox->setValue(30);
}

void MainWindow::updateMakeEnabled()
{
    QString filename = ui_->output_filename_lineedit->text();
    QString dir = ui_->output_dir_lineedit->text();
    bool enabled = ui_->buttonMakePanorama->isEnabled();

    if (enabled) {
        if (filename.isEmpty()) {
            ui_->buttonMakePanorama->setEnabled(false);
            return;
        }

        if (dir.isEmpty() || !QDir(dir).exists()) {
            ui_->buttonMakePanorama->setEnabled(false);
            return;
        }
    }
    else if (!filename.isEmpty() && !dir.isEmpty() && QDir(dir).exists())
        ui_->buttonMakePanorama->setEnabled(true);
}

void MainWindow::updateOCL()
{
    QString yes("Yes"), no("No");
    bool have_opencl = cv::ocl::haveOpenCL();
    ui_->haveopencl_value->setText(have_opencl ? yes : no);
    ui_->use_opencl_checkbox->setEnabled(have_opencl);
    ui_->use_opencl_checkbox->setChecked(false);

    if (!have_opencl) {
        ui_->opencl_device_tree->hide();
        ui_->opencl_device_tree_label->hide();
    }
    else {
        cv::ocl::Device default_device = cv::ocl::Device::getDefault();
        std::vector<cv::ocl::PlatformInfo> platforms;
        cv::ocl::getPlatfomsInfo(platforms);
        for (int i = 0; i < static_cast<int>(platforms.size()); i++) {
            const cv::ocl::PlatformInfo* platform = &platforms[i];

            QString platform_txt = QString::fromStdString(platform->name());
            QTreeWidgetItem* platform_item = new QTreeWidgetItem(
                QStringList(platform_txt));
            ui_->opencl_device_tree->insertTopLevelItem(i, platform_item);

            cv::ocl::Device current_device;
            for (int j = 0; j < platform->deviceNumber(); j++) {
                platform->getDevice(current_device, j);

                QString device_txt;
                device_txt = QString::fromStdString(current_device.name());
                device_txt += " (" + oclDeviceTypeToString(current_device.type())
                              + ")";

                QTreeWidgetItem* device_item = new QTreeWidgetItem(
                    QStringList(device_txt));
                platform_item->addChild(device_item);
                if (!current_device.available()) {
                    device_item->setDisabled(true);
                }

                if (current_device.name() == default_device.name()) {
                    device_item->setForeground(0, QBrush(Qt::green));
                }
                QString mem_type;
                if (current_device.localMemType()
                    == cv::ocl::Device::LOCAL_IS_GLOBAL)
                    mem_type = "global";
                else if (current_device.localMemType() == cv::ocl::Device::LOCAL_IS_LOCAL)
                    mem_type = "local";
                else if (current_device.localMemType() == cv::ocl::Device::NO_LOCAL_MEM)
                    mem_type = "none";
                QString tooltip;
                tooltip += QString("2D Image max size : %1x%2\n")
                               .arg(current_device.image2DMaxWidth())
                               .arg(current_device.image2DMaxWidth());
                tooltip += QString("Max mem alloc size : %1 MB\n")
                               .arg(current_device.maxMemAllocSize() / 1000000);
                tooltip += QString("Global mem size : %1 MB\n")
                               .arg(current_device.globalMemSize() / 1000000);
                tooltip += QString("Local mem type and size : %1 kB (%2)")
                               .arg(current_device.localMemSize() / 1000)
                               .arg(mem_type);
                device_item->setToolTip(0, tooltip);
            }
        }
        ui_->opencl_device_tree->expandAll();
        ui_->opencl_device_tree->show();
        ui_->opencl_device_tree_label->show();
    }
}

void MainWindow::updateEigen()
{
    QRegExp regex(".*Eigen:([ \\t]*)([^\\n\\r]*)");
    regex.indexIn(cv::getBuildInformation().c_str());
    ui_->have_eigen_value->setText(
        regex.cap(2).replace("YES", "Yes").replace("NO", "No"));
}

void MainWindow::updateIPP()
{
    QRegExp regex(".*IPP:([ \\t]*)([^\\n\\r]*)");
    regex.indexIn(cv::getBuildInformation().c_str());
    ui_->have_ipp_value->setText(
        regex.cap(2).replace("YES", "Yes").replace("NO", "No"));
}

void MainWindow::updateArch()
{
#ifdef ENVIRONMENT64
    ui_->arch_value->setText("64 bits");
#else
#ifdef ENVIRONMENT32
    ui_->arch_value->setText("32 bits");
#endif
#endif
}

void MainWindow::updateVersion()
{
    ui_->version_value->setText(APP_VERSION);
}

void MainWindow::updateOutputDirFilename()
{
    QStringList fl;
    if (ui_->selectedOnly_checkbox->isChecked())
        fl = ui_->filesListWidget->getSelectedFilesList();
    else
        fl = ui_->filesListWidget->getFilesList();

    if (!manual_output_dir_) {
        if (fl.size() > 0)
            ui_->output_dir_lineedit->setText(
                QFileInfo(fl[0]).absoluteDir().path());
        else
            ui_->output_dir_lineedit->setText("");
    }

    if (!manual_output_filename_) {
        if (fl.size() > 0) {
            QStringList basenames;
            QString new_name;
            for (int i = 0; i < fl.size(); ++i)
                basenames << QFileInfo(fl[i]).baseName();
            new_name = "pano_" + basenames.join("_");
            new_name.truncate(max_filename_length_);
            ui_->output_filename_lineedit->setText(new_name);
        }
        else
            ui_->output_filename_lineedit->setText("");
    }
}

void MainWindow::updateStatusBar()
{
    QString text = QString(
                       "Current job : %1%       Jobs left : %2      Jobs done "
                       ": %3      Jobs failed : %4")
                       .arg(getCurrentProgress())
                       .arg(getNbQueued())
                       .arg(getNbDone())
                       .arg(getNbFailed());
    ui_->statusBar->showMessage(text);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    int nb = getNbQueued();
    if (nb == 0)
        close();
    else {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
            this,
            "Quit ?",
            QString("All panoramas are not done yet. \nAre you sure you want "
                    "to quit ?"),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
            close();
        else
            event->ignore();
    }
}

void MainWindow::setupLogsTable()
{
    LogSink::setTextBrowser(ui_->logsBrowser);
    ui_->logsBrowser->setFont([] {
        QFont font("Monospace");
        font.setStyleHint(QFont::TypeWriter);
        return font;
    }());

    qInstallMessageHandler(&LogSink::onMessageOutput);
}

void MainWindow::openPostProcess(const QString& output_path)
{
    (new PostProcess(output_path, this))->show();
}

void MainWindow::startWorker(PanoramaMaker& worker)
{
    qDebug() << "Starting worker";
    progress_bars_[&worker].close->setDisabled(true);
    progress_bars_[&worker].post_process->setDisabled(true);
    worker.start();
}

void MainWindow::createWorkerUi(std::shared_ptr<PanoramaMaker> worker)
{
    std::shared_ptr<QHBoxLayout> hbox = std::make_shared<QHBoxLayout>();
    QProgressBar* progress_bar = new QProgressBar(this);
    QPushButton* close = new QPushButton("Cancel", this);
    QPushButton* post_process = new QPushButton("Post-process", this);

    progress_bar->setRange(0, 100);
    progress_bar->setFormat(worker->getOutputFilename() + " : %p%");
    progress_bar->setAlignment(Qt::AlignCenter);
    progress_bar->setValue(0);
    progress_bar->setToolTip(worker->getStitcherConfString());

    ProgressBarContent pb_struct;
    pb_struct.auto_open_post_process = ui_->post_process_checkbox->isChecked();
    pb_struct.pb = progress_bar;
    pb_struct.close = close;
    pb_struct.post_process = post_process;
    pb_struct.worker = worker;
    pb_struct.layout = hbox;

    progress_bars_[worker.get()] = pb_struct;
    progress_bars_[close] = pb_struct;

    connect(
        worker.get(),
        &PanoramaMaker::percentage,
        progress_bar,
        &QProgressBar::setValue);
    connect(worker.get(), &PanoramaMaker::percentage, this, &MainWindow::updateStatusBar);
    connect(worker.get(), &PanoramaMaker::isFailed, this, &MainWindow::onWorkerFailed);
    connect(worker.get(), &PanoramaMaker::isDone, this, &MainWindow::onWorkerDone);
    connect(worker.get(), &PanoramaMaker::finished, this, &MainWindow::updateStatusBar);

    connect(close, &QPushButton::clicked, this, &MainWindow::closeSenderWorker);

    hbox->addWidget(progress_bar);
    hbox->addWidget(post_process);
    hbox->addWidget(close);
    ui_->tabProgressLayout->addLayout(hbox.get());
}

void MainWindow::configureWorker(PanoramaMaker& worker)
{
    // OpenCL
    worker.setUseOpenCL(ui_->use_opencl_checkbox->isChecked());

    // Registration resolution
    worker.setRegistrationResol(ui_->regres_spinbox->value());

    // Feature finder mode
    worker.setFeaturesFinderMode(ui_->featuresfinder_combobox->currentText());

    // Feature matching mode and confidence
    PanoramaMaker::FeaturesMatchingMode f_matching_mode;
    f_matching_mode.mode = ui_->featuresmatcher_combobox->currentText();
    f_matching_mode.conf = ui_->featuresmatcherconf_spinbox->value();
    worker.setFeaturesMatchingMode(f_matching_mode);

    // Warp mode
    worker.setWarpMode(ui_->warpmode_combobox->currentText());

    // Wave correction
    worker.setWaveCorrectionMode(ui_->wavecorkind_combobox->currentText());

    // Bundle adjuster
    worker.setBundleAdjusterMode(ui_->bundleadj_combobox->currentText());

    // Panorama confidence
    worker.setPanoConfidenceThresh(ui_->confth_spinbox->value());

    // Exposure compensator mode
    PanoramaMaker::ExposureComensatorMode exp_comp_mode;
    exp_comp_mode.mode = ui_->expcomp_mode_combobox->currentText();
    exp_comp_mode.type = ui_->expcomp_type_combobox->currentText();
    exp_comp_mode.block_size = ui_->blocksize_spinbox->value();
    exp_comp_mode.nfeed = ui_->nfeed_spinbox->value();
    exp_comp_mode.similarity_th = ui_->exp_sim_th_spinbox->value();
    worker.setExposureCompensatorMode(exp_comp_mode);

    // Seam estimation resolution
    worker.setSeamEstimationResol(ui_->seamfinderres_spinbox->value());

    // Seam finder mode
    worker.setSeamFinderMode(ui_->seamfindermode_combobox->currentText());

    // Blender
    PanoramaMaker::BlenderMode blender_mode;
    blender_mode.mode = ui_->blendertype_combobox->currentText();
    blender_mode.sharpness = ui_->sharpness_spinbox->value();
    blender_mode.bands = ui_->nbands_spinbox->value();
    worker.setBlenderMode(blender_mode);

    // Compositing resolution
    double compositing_res = ui_->compositingres_spinbox->value();
    if (compositing_res <= 0)
        compositing_res = cv::Stitcher::ORIG_RESOL;

    worker.setCompositingResol(compositing_res);

    // Interpolation
    worker.setInterpolationMode(ui_->interp_combobox->currentText());

    // Images per video
    worker.setImagesPerVideo(ui_->images_per_videos_spinbox->value());
}

void MainWindow::closeSenderWorker()
{
    ProgressBarContent& pb_struct = progress_bars_[QObject::sender()];
    progress_bars_.erase(pb_struct.close);
    progress_bars_.erase(pb_struct.worker.get());
    pb_struct.layout.reset();
    workers_.removeAll(pb_struct.worker);
    pb_struct.worker.reset();
    updateStatusBar();
}

QString MainWindow::oclDeviceTypeToString(int type)
{
    QStringList strs;
    if (type & cv::ocl::Device::TYPE_CPU)
        strs << "CPU";
    if (type & cv::ocl::Device::TYPE_GPU) {
        bool type_found = false;
        if ((type - cv::ocl::Device::TYPE_GPU) & cv::ocl::Device::TYPE_DGPU) {
            type_found = true;
            strs << "DGPU";
        }
        if ((type - cv::ocl::Device::TYPE_GPU) & cv::ocl::Device::TYPE_IGPU) {
            type_found = true;
            strs << "IGPU";
        }
        if (!type_found) {
            strs << "GPU";
        }
    }
    if (type & cv::ocl::Device::TYPE_ACCELERATOR)
        strs << "Accelerator";

    if (strs.size() == 0)
        return QString("Unknown type");

    return strs.join(", ");
}

} // autopanorama
