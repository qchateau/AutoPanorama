#ifndef QFILEWIDGET_H
#define QFILEWIDGET_H

#include <QListWidget>
#include <QStringList>
#include <QObject>

class QFileWidget : public QListWidget
{
    Q_OBJECT
public:
    QFileWidget(QWidget *parent=0);

    void addFiles(QStringList files);
    QStringList getFilesList();
    QStringList getSelectedFilesList();

    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dropEvent(QDropEvent *);

public slots:
    void selectAndAddFiles();
    void removeSelected();

signals:
    void filesDropped(QStringList files);

protected:
    bool eventFilter(QObject* obj, QEvent* event);

private:
    QPoint drag_start_position;
    QDrag *drag;
};

#endif // QFILEWIDGET_H
