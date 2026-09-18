/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionAND;
    QAction *actionNOT;
    QAction *actionOR;
    QAction *actionNAND;
    QAction *actionNOR;
    QWidget *centralwidget;
    QGraphicsView *graphicsView;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1211, 841);
        actionAND = new QAction(MainWindow);
        actionAND->setObjectName("actionAND");
        actionNOT = new QAction(MainWindow);
        actionNOT->setObjectName("actionNOT");
        actionOR = new QAction(MainWindow);
        actionOR->setObjectName("actionOR");
        actionNAND = new QAction(MainWindow);
        actionNAND->setObjectName("actionNAND");
        actionNOR = new QAction(MainWindow);
        actionNOR->setObjectName("actionNOR");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        graphicsView = new QGraphicsView(centralwidget);
        graphicsView->setObjectName("graphicsView");
        graphicsView->setGeometry(QRect(0, 0, 1221, 831));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1211, 24));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actionAND->setText(QCoreApplication::translate("MainWindow", "AND", nullptr));
        actionNOT->setText(QCoreApplication::translate("MainWindow", "NOT", nullptr));
        actionOR->setText(QCoreApplication::translate("MainWindow", "OR", nullptr));
        actionNAND->setText(QCoreApplication::translate("MainWindow", "NAND", nullptr));
        actionNOR->setText(QCoreApplication::translate("MainWindow", "NOR", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
