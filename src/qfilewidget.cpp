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

QFileWidget::QFileWidget(QWidget *parent) :
    QListWidget(parent)
{
    installEventFilter(this);
}

void QFileWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QListWidget::dragEnterEvent(event);

    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls())
    {
        event->acceptProposedAction();
    }
}

void QFileWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QListWidget::dragMoveEvent(event);

    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

void QFileWidget::dropEvent(QDropEvent *event)
{
    QListWidget::dropEvent(event);

    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls())
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
            addFiles(new_files);
            emit filesDropped(new_files);
        }
    }
}

void QFileWidget::addFiles(QStringList files)
{
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
}

void QFileWidget::selectAndAddFiles()
{
    QString filter = "Images (*.png *.jpg *.bmp)";
    QStringList files = QFileDialog::getOpenFileNames(Q_NULLPTR, QString(), QString(), filter);
    addFiles(files);
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
