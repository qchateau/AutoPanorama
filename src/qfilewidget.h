#ifndef QFILEWIDGET_H
#define QFILEWIDGET_H

#include <QListWidget>
#include <QObject>
#include <QStringList>
#include <QTimer>

namespace autopanorama {

class QFileWidget : public QListWidget {
    Q_OBJECT
public:
    QFileWidget(QWidget* parent = 0);

    void addSupportedExtension(QString ext)
    {
        supported_extensions << ext.toLower();
    }
    void addFiles(QStringList files);
    void asyncAddFiles(QStringList files);
    QStringList getFilesList();
    QStringList getSelectedFilesList();
    int countActive();

public slots:
    void cleanItems();
    void clear();
    void removeSelected();
    void selectAndAddFiles();

protected:
    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void dropEvent(QDropEvent*);
    bool eventFilter(QObject* obj, QEvent* event);
    void paintEvent(QPaintEvent* e);

    void remove(int i);
    void remove(QListWidgetItem* item);

private:
    QIcon default_icon, no_preview_icon, video_icon;
    QTimer items_cleaner;
    QPoint drag_start_position;
    QDrag* drag;
    QList<QListWidgetItem*> items_to_delete;
    QStringList supported_extensions;

    void refreshIcons();

signals:
    void iconChanged();
    void itemsAdded();
    void itemsRemoved();
};

} // autopanorama

#endif // QFILEWIDGET_H