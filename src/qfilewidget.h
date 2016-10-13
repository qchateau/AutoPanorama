#ifndef QFILEWIDGET_H
#define QFILEWIDGET_H

#include <QObject>
#include <QListWidget>
#include <QTimer>

class QFileWidget : public QListWidget
{
    Q_OBJECT
public:
    QFileWidget(QWidget *parent=0);

    void addFiles(QStringList files);
    void asyncAddFiles(QStringList files);
    QStringList getFilesList();
    QStringList getSelectedFilesList();

    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dropEvent(QDropEvent *);

public slots:
    void clean_items();
    void clear();
    void selectAndAddFiles();
    void removeSelected();

signals:
    void filesDropped(QStringList files);

protected:
    void remove(int i);
    void remove(QListWidgetItem* item);
    bool eventFilter(QObject* obj, QEvent* event);

private:
    QIcon default_icon;
    QTimer items_cleaner;
    QPoint drag_start_position;
    QDrag *drag;
    QList<QListWidgetItem*> items_to_delete;
};

#endif // QFILEWIDGET_H
