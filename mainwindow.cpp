#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPainter>
#include <QToolBar>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);

    scene->setSceneRect(0, 0, 1200, 700);

    ui->graphicsView->setScene(scene);

    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    QToolBar *toolbar = addToolBar("Gate Toolbar");

    toolbar->addAction(ui->actionAND);
    toolbar->addAction(ui->actionOR);
    toolbar->addAction(ui->actionNOT);
    toolbar->addAction(ui->actionNAND);
    toolbar->addAction(ui->actionNOR);

    
}

MainWindow::~MainWindow()
{
    delete ui;
}