#include "qfilewidget.h"

#include <QtDebug>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QStringList>
#include <QList>
#include <QMouseEvent>
#include <QFileDialog>
#include <QDrag>
#include <QApplication>
#include <QtConcurrent/QtConcurrentRun>

QFileWidget::QFileWidget(QWidget *parent) :
    QListWidget(parent)
{
    installEventFilter(this);
}

void QFileWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QListWidget::dragEnterEvent(event);

    const QMimeData *mimeData = event->mimeData();
    if (event->source() == this)
    {
        event->ignore();
    }
    else if (mimeData->hasUrls() && (event->source() == 0))
    {
        event->acceptProposedAction();
    }
}

void QFileWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QListWidget::dragMoveEvent(event);

    if (event->mimeData()->hasUrls() && (event->source() == 0))
    {
        event->acceptProposedAction();
    }
}

void QFileWidget::dropEvent(QDropEvent *event)
{
    QListWidget::dropEvent(event);

    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls() && (event->source() == 0))
    {
        QStringList new_files;
        QList<QUrl> urlList = mimeData->urls();
        for (int i = 0; i < urlList.size() && i < 32; ++i)
        {
            QUrl &url = urlList[i];
            if (url.isLocalFile())
            {
                new_files << url.path();
            }
        }
        if (new_files.size() > 0) {
            event->acceptProposedAction();
            asyncAddFiles(new_files);
            emit filesDropped(new_files);
        }
    }
}

void QFileWidget::asyncAddFiles(QStringList files)
{
    QtConcurrent::run(this, &QFileWidget::addFiles, files);
}

void QFileWidget::addFiles(QStringList files)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (int i = 0; i < files.size(); ++i)
    {
        QFileInfo fileinfo(files[i]);
        if (fileinfo.exists() && fileinfo.isFile() && (!getFilesList().contains(fileinfo.absoluteFilePath())))
        {
            QIcon small_icon, big_icon(fileinfo.absoluteFilePath());
            small_icon = QIcon(big_icon.pixmap(iconSize()));
            QListWidgetItem *new_item = new QListWidgetItem(small_icon, fileinfo.fileName(), this);
            new_item->setData(Qt::ToolTipRole, QVariant(fileinfo.absoluteFilePath()));
        }
    }
    QApplication::restoreOverrideCursor();
}

void QFileWidget::selectAndAddFiles()
{
    QString filter = "Images (*.png *.jpg *.bmp)";
    QStringList files = QFileDialog::getOpenFileNames(Q_NULLPTR, QString(), QString(), filter);
    asyncAddFiles(files);
}

void QFileWidget::removeSelected()
{
    QList<QListWidgetItem*> selected = selectedItems();
    for (int i = 0; i < selected.size(); ++i)
    {
        delete selected[i];
    }
}

QStringList QFileWidget::getFilesList()
{
    QStringList files_list;
    for (int i = 0; i < count(); ++i)
    {
        files_list << item(i)->data(Qt::ToolTipRole).toString();
    }
    return files_list;
}

QStringList QFileWidget::getSelectedFilesList()
{
    QStringList files_list;
    QList<QListWidgetItem*> selected = selectedItems();
    for (int i = 0; i < selected.size(); ++i)
    {
        files_list << selected[i]->data(Qt::ToolTipRole).toString();
    }
    return files_list;
}

bool QFileWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type()==QEvent::KeyPress)
    {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if (key->key()==Qt::Key_Delete)
        {
            removeSelected();
        }
        else
        {
            return QListWidget::eventFilter(obj, event);
        }
        return true;
        }
    else
    {
        return QListWidget::eventFilter(obj, event);
    }
    return false;
}
