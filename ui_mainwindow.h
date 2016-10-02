/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionQuit;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QTabWidget *mainWidget;
    QWidget *tabMain;
    QVBoxLayout *verticalLayout;
    QTreeView *fsView;
    QHBoxLayout *layoutButtons;
    QPushButton *buttonSelectDir;
    QPushButton *buttonMakePanorama;
    QWidget *tabProgress;
    QVBoxLayout *verticalLayout_3;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_4;
    QVBoxLayout *tabProgressLayout;
    QSpacerItem *verticalSpacer;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(800, 600);
        actionQuit = new QAction(MainWindow);
        actionQuit->setObjectName(QStringLiteral("actionQuit"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        mainWidget = new QTabWidget(centralWidget);
        mainWidget->setObjectName(QStringLiteral("mainWidget"));
        tabMain = new QWidget();
        tabMain->setObjectName(QStringLiteral("tabMain"));
        verticalLayout = new QVBoxLayout(tabMain);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        fsView = new QTreeView(tabMain);
        fsView->setObjectName(QStringLiteral("fsView"));
        fsView->setSelectionMode(QAbstractItemView::MultiSelection);
        fsView->header()->setStretchLastSection(false);

        verticalLayout->addWidget(fsView);

        layoutButtons = new QHBoxLayout();
        layoutButtons->setSpacing(6);
        layoutButtons->setObjectName(QStringLiteral("layoutButtons"));
        buttonSelectDir = new QPushButton(tabMain);
        buttonSelectDir->setObjectName(QStringLiteral("buttonSelectDir"));

        layoutButtons->addWidget(buttonSelectDir);

        buttonMakePanorama = new QPushButton(tabMain);
        buttonMakePanorama->setObjectName(QStringLiteral("buttonMakePanorama"));

        layoutButtons->addWidget(buttonMakePanorama);


        verticalLayout->addLayout(layoutButtons);

        mainWidget->addTab(tabMain, QString());
        tabProgress = new QWidget();
        tabProgress->setObjectName(QStringLiteral("tabProgress"));
        verticalLayout_3 = new QVBoxLayout(tabProgress);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setSizeConstraint(QLayout::SetDefaultConstraint);
        scrollArea = new QScrollArea(tabProgress);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        scrollArea->setWidgetResizable(true);
        scrollArea->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 756, 495));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(scrollAreaWidgetContents->sizePolicy().hasHeightForWidth());
        scrollAreaWidgetContents->setSizePolicy(sizePolicy);
        scrollAreaWidgetContents->setMinimumSize(QSize(0, 0));
        scrollAreaWidgetContents->setAutoFillBackground(true);
        verticalLayout_4 = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        verticalLayout_4->setSizeConstraint(QLayout::SetDefaultConstraint);
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        tabProgressLayout = new QVBoxLayout();
        tabProgressLayout->setSpacing(6);
        tabProgressLayout->setObjectName(QStringLiteral("tabProgressLayout"));

        verticalLayout_4->addLayout(tabProgressLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);

        scrollArea->setWidget(scrollAreaWidgetContents);

        verticalLayout_3->addWidget(scrollArea);

        mainWidget->addTab(tabProgress, QString());

        verticalLayout_2->addWidget(mainWidget);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 800, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionQuit);

        retranslateUi(MainWindow);
        QObject::connect(actionQuit, SIGNAL(triggered()), MainWindow, SLOT(close()));
        QObject::connect(buttonSelectDir, SIGNAL(clicked()), MainWindow, SLOT(onSelectDirClicked()));
        QObject::connect(buttonMakePanorama, SIGNAL(clicked()), MainWindow, SLOT(onMakePanoramaClicked()));

        mainWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        actionQuit->setText(QApplication::translate("MainWindow", "Quit", 0));
        buttonSelectDir->setText(QApplication::translate("MainWindow", "Select source directory", 0));
        buttonMakePanorama->setText(QApplication::translate("MainWindow", "Make panorama", 0));
        mainWidget->setTabText(mainWidget->indexOf(tabMain), QApplication::translate("MainWindow", "Files", 0));
        mainWidget->setTabText(mainWidget->indexOf(tabProgress), QApplication::translate("MainWindow", "Progress", 0));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
