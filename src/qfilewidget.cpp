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
#include <QIcon>
#include <QPixmap>
#include "ui_mainwindow.h"

QFileWidget::QFileWidget(QWidget *parent) :
    QListWidget(parent),
    items_cleaner(this)
{
    installEventFilter(this);

    default_icon = QIcon(":/icon_loading.png");
    items_cleaner.moveToThread(QApplication::instance()->thread());
    connect(&items_cleaner, SIGNAL(timeout()), this, SLOT(clean_items()));
    items_cleaner.start(5000);
}

void QFileWidget::addFiles(QStringList files)
{
    QList<QListWidgetItem*> added_items;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (int i = 0; i < files.size(); ++i)
    {
        QFileInfo fileinfo(files[i]);
        if (fileinfo.exists() && fileinfo.isFile() &&
                (supported_extensions.contains(fileinfo.suffix().toLower())) &&
                (!getFilesList().contains(fileinfo.absoluteFilePath())))
        {
            QListWidgetItem *new_item = new QListWidgetItem(default_icon, fileinfo.fileName(), this);
            new_item->setToolTip(fileinfo.absoluteFilePath());
            added_items << new_item;
        }
    }
    QApplication::restoreOverrideCursor();
    emit itemsAdded();

    // Delayed icon set
    for (int i = 0; i < added_items.size(); ++i)
    {
        if (items_to_delete.contains(added_items[i]))
        {
            // Change the icon so item will be deleted
            // but don't spend time loading the full image
            added_items[i]->setIcon(QIcon());
        }
        else
        {
            QIcon small_icon, big_icon(added_items[i]->toolTip());
            small_icon = QIcon(big_icon.pixmap(iconSize()));
            added_items[i]->setIcon(small_icon);
            scheduleDelayedItemsLayout();
        }
    }
}

void QFileWidget::asyncAddFiles(QStringList files)
{
    QtConcurrent::run(this, &QFileWidget::addFiles, files);
}

QStringList QFileWidget::getFilesList()
{
    QStringList files_list;
    for (int i = 0; i < count(); ++i)
    {
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
    for (int i = 0; i < selected.size(); ++i)
    {
        if (selected[i]->isHidden())
            continue;
        files_list << selected[i]->data(Qt::ToolTipRole).toString();
    }
    return files_list;
}








void QFileWidget::clean_items()
{
    if (thread() != QApplication::instance()->thread())
    {
        qDebug() << "WARNING : clean_item must be called in the GUI thread. Skipping.";
        return;
    }

    QList<QListWidgetItem*> items_not_deleted;
    while (items_to_delete.size() > 0)
    {
        QListWidgetItem* item = items_to_delete.takeFirst();
        if (item)
        {
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
        filters << "*."+supported_extensions[i];
    QString filter = QString("Images (%1)").arg(filters.join(' '));
    QStringList files = QFileDialog::getOpenFileNames(Q_NULLPTR, QString(), QString(), filter);
    asyncAddFiles(files);
}







void QFileWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QListWidget::dragEnterEvent(event);

    const QMimeData *mimeData = event->mimeData();
    if (event->source() == this)
        event->ignore();
    else if (mimeData->hasUrls() && (event->source() == 0))
        event->acceptProposedAction();
}

void QFileWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QListWidget::dragMoveEvent(event);

    if (event->mimeData()->hasUrls() && (event->source() == 0))
        event->acceptProposedAction();
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
                new_files << url.toLocalFile();
        }
        if (new_files.size() > 0)
        {
            event->acceptProposedAction();
            asyncAddFiles(new_files);
        }
    }
}

bool QFileWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type()==QEvent::KeyPress)
    {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if (key->key()==Qt::Key_Delete)
            removeSelected();
        else
            return QListWidget::eventFilter(obj, event);

        return true;
    }
    else
        return QListWidget::eventFilter(obj, event);

    return false;
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
