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

public slots:
    void clean_items();
    void clear();
    void removeSelected();
    void selectAndAddFiles();

protected:
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dropEvent(QDropEvent *);
    bool eventFilter(QObject* obj, QEvent* event);

    void remove(int i);
    void remove(QListWidgetItem* item);

private:
    QIcon default_icon;
    QTimer items_cleaner;
    QPoint drag_start_position;
    QDrag *drag;
    QList<QListWidgetItem*> items_to_delete;

signals:
    void itemsAdded();
    void itemsRemoved();
};

#endif // QFILEWIDGET_H
