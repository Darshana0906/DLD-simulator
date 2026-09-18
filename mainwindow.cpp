#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gates/AndGate.h"
#include "gates/OrGate.h"
#include "gates/NotGate.h"
#include "gates/NandGate.h"
#include "gates/NorGate.h"
#include "gates/XorGate.h"
#include "gates/XnorGate.h"
#include "elements/Point.h"
#include "elements/Wire.h"
#include "elements/LED.h"
#include "elements/GND.h"
#include "elements/Const1.h"
#include "elements/Switch.h"
#include <QToolBar>
#include <QAction>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    scene = new QGraphicsScene(this);

    scene->setSceneRect(0, 0, 1200, 700);

    ui->graphicsView->setScene(scene);

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

    QAction *pointAction = toolbar->addAction("Point");
    QAction *wireAction = toolbar->addAction("Wire");
    QAction *ledAction = toolbar->addAction("LED");
    QAction *gndAction = toolbar->addAction("GND");
    QAction *constantAction = toolbar->addAction("1");
    QAction *switchAction = toolbar->addAction("Switch");
        



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

    connect(pointAction, &QAction::triggered, this, [this]() {
        Point *point = new Point();
        point->setPos(300, 200);
        scene->addItem(point);
    });

    connect(wireAction, &QAction::triggered, this, [this]() {
        Point *start = new Point();
        Point *end = new Point();
        start->setPos(300, 200);
        end->setPos(500, 200);
        scene->addItem(start);
        scene->addItem(end);
        Wire *wire = new Wire(start, end);
        scene->addItem(wire);
    });
    connect(ledAction, &QAction::triggered, this, [this]() {
        LED *led = new LED();
        led->setPos(300, 200);
        scene->addItem(led);
    });
    connect(gndAction, &QAction::triggered, this, [this]() {
        GND *gnd = new GND();
        gnd->setPos(300, 200);
        scene->addItem(gnd);
    });
    connect(constantAction, &QAction::triggered, this, [this]() {
        Const1 *constant = new Const1();
        constant->setPos(300, 200);
        scene->addItem(constant);
    });
    connect(switchAction, &QAction::triggered, this, [this]() {
        Switch *sw = new Switch();
        sw->setPos(300, 200);
        scene->addItem(sw);
    });
    
}

MainWindow::~MainWindow() {
    delete ui;
}