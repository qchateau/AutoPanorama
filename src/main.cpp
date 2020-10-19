#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
#ifdef _WIN32
    a.setStyle(QStyleFactory::create("fusion"));
#endif

    MainWindow w;
    w.show();

    return a.exec();
}
