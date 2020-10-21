#include "qfilewidget.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QImageReader>
#include <QList>
#include <QListWidgetItem>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QStringList>
#include <QtDebug>
#include <QtConcurrent/QtConcurrentRun>

namespace autopanorama {

QFileWidget::QFileWidget(QWidget* parent)
    : QListWidget(parent), items_cleaner(this)
{
    installEventFilter(this);

    default_icon = QIcon(":/icon_loading.png");
    no_preview_icon = QIcon(":/icon_invalid.png");
    video_icon = QIcon(":/icon_video.png");
    connect(this, &QFileWidget::iconChanged, this, &QFileWidget::refreshIcons);
    connect(&items_cleaner, &QTimer::timeout, this, &QFileWidget::cleanItems);
    items_cleaner.start(5000);
    items_cleaner.moveToThread(QApplication::instance()->thread());
}

void QFileWidget::addFiles(QStringList files)
{
    QList<QListWidgetItem*> added_items;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (int i = 0; i < files.size(); ++i) {
        QFileInfo fileinfo(files[i]);
        QString extension = fileinfo.suffix().toLower();
        if (fileinfo.exists() && fileinfo.isFile()
            && (supported_extensions.empty()
                || supported_extensions.contains(extension))
            && (!getFilesList().contains(fileinfo.absoluteFilePath()))) {
            QListWidgetItem* new_item =
                new QListWidgetItem(default_icon, fileinfo.fileName(), this);
            new_item->setToolTip(fileinfo.absoluteFilePath());
            added_items << new_item;
        }
    }
    QApplication::restoreOverrideCursor();
    emit itemsAdded();

    // Delayed icon set
    for (int i = 0; i < added_items.size(); ++i) {
        if (items_to_delete.contains(added_items[i])) {
            // Change the icon so item will be deleted
            // but don't spend time loading the full image
            added_items[i]->setIcon(QIcon());
        }
        else {
            QString filename = added_items[i]->toolTip();
            QString ext = QFileInfo(filename).suffix().toLower();
            QStringList videos({"mp4", "avi", "mov", "mkv", "mpeg"});
            QIcon small_icon;
            QImageReader reader(filename);
            if (reader.canRead()) {
                QPixmap pixmap = QPixmap::fromImageReader(&reader);
                QIcon big_icon(pixmap);
                small_icon = QIcon(big_icon.pixmap(iconSize()));
            }
            else if (videos.contains(ext)) {
                small_icon = video_icon;
            }
            else {
                small_icon = no_preview_icon;
            }
            added_items[i]->setIcon(small_icon);
        }
    }
    emit iconChanged();
}

void QFileWidget::asyncAddFiles(QStringList files)
{
    QtConcurrent::run(this, &QFileWidget::addFiles, files);
}

QStringList QFileWidget::getFilesList()
{
    QStringList files_list;
    for (int i = 0; i < count(); ++i) {
        if (item(i)->isHidden())
            continue;
        files_list << item(i)->data(Qt::ToolTipRole).toString();
    }
    return files_list;
}

QStringList QFileWidget::getSelectedFilesList()
{
    QStringList files_list;
    QList<QListWidgetItem*> selected = selectedItems();
    for (int i = 0; i < selected.size(); ++i) {
        if (selected[i]->isHidden())
            continue;
        files_list << selected[i]->data(Qt::ToolTipRole).toString();
    }
    return files_list;
}

void QFileWidget::refreshIcons()
{
    scheduleDelayedItemsLayout();
}

void QFileWidget::cleanItems()
{
    if (thread() != QApplication::instance()->thread()) {
        qDebug() << "WARNING : clean_item must be called in the GUI thread. "
                    "Skipping.";
        return;
    }

    QList<QListWidgetItem*> items_not_deleted;
    while (items_to_delete.size() > 0) {
        QListWidgetItem* item = items_to_delete.takeFirst();
        if (item) {
            if (item->icon().cacheKey() == default_icon.cacheKey())
                items_not_deleted << item;
            else
                delete item;
        }
    }
    items_to_delete = items_not_deleted;
}

void QFileWidget::clear()
{
    for (int i = 0; i < count(); ++i)
        remove(i);
}

void QFileWidget::removeSelected()
{
    QList<QListWidgetItem*> selected = selectedItems();
    clearSelection();
    for (int i = 0; i < selected.size(); ++i)
        remove(selected[i]);
}

void QFileWidget::selectAndAddFiles()
{
    QStringList filters;
    for (int i = 0; i < supported_extensions.size(); ++i)
        filters << "*." + supported_extensions[i];
    QString filter = QString("Images/Videos (%1)").arg(filters.join(' '));
    QStringList files =
        QFileDialog::getOpenFileNames(Q_NULLPTR, QString(), QString(), filter);
    asyncAddFiles(files);
}

int QFileWidget::countActive()
{
    int cnt = 0;
    for (int i = 0; i < count(); ++i)
        if (!item(i)->isHidden())
            ++cnt;
    return cnt;
}

void QFileWidget::dragEnterEvent(QDragEnterEvent* event)
{
    QListWidget::dragEnterEvent(event);

    const QMimeData* mimeData = event->mimeData();
    if (event->source() == this)
        event->ignore();
    else if (mimeData->hasUrls() && (event->source() == 0))
        event->acceptProposedAction();
}

void QFileWidget::dragMoveEvent(QDragMoveEvent* event)
{
    QListWidget::dragMoveEvent(event);

    if (event->mimeData()->hasUrls() && (event->source() == 0))
        event->acceptProposedAction();
}

void QFileWidget::dropEvent(QDropEvent* event)
{
    QListWidget::dropEvent(event);

    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls() && (event->source() == 0)) {
        QStringList new_files;
        QList<QUrl> urlList = mimeData->urls();
        for (int i = 0; i < urlList.size() && i < 32; ++i) {
            QUrl& url = urlList[i];
            if (url.isLocalFile())
                new_files << url.toLocalFile();
        }
        if (new_files.size() > 0) {
            event->acceptProposedAction();
            asyncAddFiles(new_files);
        }
    }
}

bool QFileWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if (key->key() == Qt::Key_Delete)
            removeSelected();
        else
            return QListWidget::eventFilter(obj, event);

        return true;
    }
    else
        return QListWidget::eventFilter(obj, event);

    return false;
}

void QFileWidget::paintEvent(QPaintEvent* e)
{
    QListWidget::paintEvent(e);
    if (countActive() > 0)
        return;

    QPainter p(this->viewport());
    p.drawText(rect(), Qt::AlignCenter, "Drop images or videos here");
}

void QFileWidget::remove(int row)
{
    remove(item(row));
}

void QFileWidget::remove(QListWidgetItem* item)
{
    item->setHidden(true);
    items_to_delete << item;
    emit itemsRemoved();
}

} // autopanorama
