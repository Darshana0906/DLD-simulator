#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create the graphics scene
    scene = new QGraphicsScene(this);

    // Attach the scene to the graphics view
    ui->graphicsView->setScene(scene);

    // Set the size of our canvas
    scene->setSceneRect(0, 0, 1200, 700);

    // Enable smooth rendering
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
}

MainWindow::~MainWindow()
{
    delete ui;
}