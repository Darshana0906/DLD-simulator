#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gates/AndGate.h"
#include "gates/OrGate.h"
#include "gates/NotGate.h"
#include "gates/NandGate.h"
#include "gates/NorGate.h"
#include "gates/XorGate.h"
#include "gates/XnorGate.h"
#include <QToolBar>
#include <QAction>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Create the graphics scene
    scene = new QGraphicsScene(this);

    // Attach the scene to the graphics view
    ui->graphicsView->setScene(scene);

    // Set the size of our canvas
    scene->setSceneRect(0, 0, 1200, 700);

    // Enable smooth rendering
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    QToolBar *toolbar = new QToolBar("Gates", this);
    addToolBar(Qt::LeftToolBarArea, toolbar);

    QAction *andAction = toolbar->addAction("AND");
    QAction *orAction = toolbar->addAction("OR");
    QAction *notAction = toolbar->addAction("NOT");
    QAction *nandAction = toolbar->addAction("NAND");
    QAction *norAction = toolbar->addAction("NOR");
    QAction *xorAction = toolbar->addAction("XOR");
    QAction *xnorAction = toolbar->addAction("XNOR");

    //connections
    connect(andAction, &QAction::triggered, this, [this]() {
    AndGate *gate = new AndGate();
    gate->setPos(300, 200);
    scene->addItem(gate);});

    connect(orAction, &QAction::triggered, this, [this]() {
    OrGate *gate = new OrGate();
    gate->setPos(300, 200);
    scene->addItem(gate);});

    connect(notAction, &QAction::triggered, this, [this]() {
    NotGate *gate = new NotGate();
    gate->setPos(300, 200);
    scene->addItem(gate);});

    connect(nandAction, &QAction::triggered, this, [this]() {
    NandGate *gate = new NandGate();
    gate->setPos(300, 200);
    scene->addItem(gate);});

    connect(norAction, &QAction::triggered, this, [this]() {
    NorGate *gate = new NorGate();
    gate->setPos(300, 200);
    scene->addItem(gate);});

    connect(xorAction, &QAction::triggered, this, [this]() {
    XorGate *gate = new XorGate();
    gate->setPos(300, 200);
    scene->addItem(gate);});

    connect(xnorAction, &QAction::triggered, this, [this]() {
    XnorGate *gate = new XnorGate();
    gate->setPos(300, 200);
    scene->addItem(gate);});
    
}

MainWindow::~MainWindow() {
    delete ui;
}