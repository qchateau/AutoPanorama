#ifndef QFILEWIDGET_H
#define QFILEWIDGET_H

#include <QListWidget>
#include <QObject>
#include <QStringList>
#include <QTimer>

namespace autopanorama {

class QFileWidget : public QListWidget {
    Q_OBJECT
signals:
    void iconChanged();
    void itemsAdded();
    void itemsRemoved();

public:
    QFileWidget(QWidget* parent = 0);

    void addSupportedExtension(QString ext)
    {
        supported_extensions_ << ext.toLower();
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
    void refreshIcons();

    QIcon default_icon_;
    QIcon no_preview_icon_;
    QIcon video_icon_;
    QTimer items_cleaner_;
    QPoint drag_start_position_;
    QDrag* drag_;
    QList<QListWidgetItem*> items_to_delete_;
    QStringList supported_extensions_;
};

} // autopanorama

#endif // QFILEWIDGET_H