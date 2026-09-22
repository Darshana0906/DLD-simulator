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
#include <QToolButton>
#include <QIcon>
#include <QSize>
#include <QLabel>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    scene = new QGraphicsScene(this);

    scene->setSceneRect(0, 0, 1200, 700);

    ui->graphicsView->setScene(scene);

    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    QToolBar *toolbar = new QToolBar("Toolbar", this);
    addToolBar(Qt::LeftToolBarArea, toolbar);
    toolbar->setIconSize(QSize(96, 64));
    
    //gates
    QFont font;
    font.setPointSize(12);


    QLabel *gatelabel = new QLabel("GATES");
    gatelabel->setAlignment(Qt :: AlignCenter);
    toolbar->addWidget(gatelabel);
    gatelabel->setFont(font);
    gatelabel->setStyleSheet("background-color: lightgray;");

    QAction *andAction = new QAction(this);
    andAction->setIcon(QIcon(":/icons/ANDgate.png"));
    QToolButton *andButton = new QToolButton();
    andButton->setDefaultAction(andAction);
    andButton->setToolTip("AND Gate");
    andButton->setFixedSize(96, 64);
    toolbar->addWidget(andButton);

    QAction *orAction = new QAction(this);
    orAction->setIcon(QIcon(":/icons/ORgate.png"));
    QToolButton *orButton = new QToolButton();
    orButton->setDefaultAction(orAction);
    orButton->setToolTip("OR Gate");
    orButton->setFixedSize(96, 64);
    toolbar->addWidget(orButton);

    QAction *notAction = new QAction(this);
    notAction->setIcon(QIcon(":/icons/NOTgate.png"));
    QToolButton *notButton = new QToolButton();
    notButton->setDefaultAction(notAction);
    notButton->setToolTip("NOT Gate");
    notButton->setFixedSize(96, 64);
    toolbar->addWidget(notButton);

    QAction *nandAction = new QAction(this);
    nandAction->setIcon(QIcon(":/icons/NANDgate.png"));
    QToolButton *nandButton = new QToolButton();
    nandButton->setDefaultAction(nandAction);
    nandButton->setToolTip("NAND Gate");
    nandButton->setFixedSize(96, 64);
    toolbar->addWidget(nandButton);

    QAction *norAction = new QAction(this);
    norAction->setIcon(QIcon(":/icons/NORgate.png"));
    QToolButton *norButton = new QToolButton();
    norButton->setDefaultAction(norAction);
    norButton->setToolTip("NOR Gate");
    norButton->setFixedSize(96, 64);
    toolbar->addWidget(norButton);

    QAction *xorAction = new QAction(this);
    xorAction->setIcon(QIcon(":/icons/XORgate.png"));
    QToolButton *xorButton = new QToolButton();
    xorButton->setDefaultAction(xorAction);
    xorButton->setToolTip("XOR Gate");
    xorButton->setFixedSize(96, 64);
    toolbar->addWidget(xorButton);

    QAction *xnorAction = new QAction(this);
    xnorAction->setIcon(QIcon(":/icons/XNORgate.png"));
    QToolButton *xnorButton = new QToolButton();
    xnorButton->setDefaultAction(xnorAction);
    xnorButton->setToolTip("XNOR Gate");
    xnorButton->setFixedSize(96, 64);
    toolbar->addWidget(xnorButton);

    //QAction *pointAction = toolbar->addAction("Point");
    QLabel *connectionlabel = new QLabel("CONNECTION");
    connectionlabel->setAlignment(Qt :: AlignCenter);
    toolbar->addWidget(connectionlabel);
    connectionlabel->setFont(font);
    connectionlabel->setStyleSheet("background-color: lightgray;");

    QAction *wireAction = new QAction(this);
    wireAction->setIcon(QIcon(":/icons/WIRE.png"));
    QToolButton *wireButton = new QToolButton();
    wireButton->setDefaultAction(wireAction);
    wireButton->setToolTip("WIRE");
    wireButton->setFixedSize(96, 64);
    toolbar->addWidget(wireButton);
    

    QLabel *inputlabel = new QLabel("INPUT");
    inputlabel->setAlignment(Qt :: AlignCenter);
    toolbar->addWidget(inputlabel);
    inputlabel->setFont(font);
    inputlabel->setStyleSheet("background-color: lightgray;");

    QAction *gndAction = new QAction(this);
    gndAction->setIcon(QIcon(":/icons/GNDip.png"));
    QToolButton *gndButton = new QToolButton();
    gndButton->setDefaultAction(gndAction);
    gndButton->setToolTip("GND");
    gndButton->setFixedSize(96, 64);
    toolbar->addWidget(gndButton);

    QAction *constantAction = new QAction(this);
    constantAction->setIcon(QIcon(":/icons/CONST1ip.png"));
    QToolButton *constantButton = new QToolButton();
    constantButton->setDefaultAction(constantAction);
    constantButton->setToolTip("CONSTANT 1");
    constantButton->setFixedSize(96, 64);
    toolbar->addWidget(constantButton);

    QAction *switchAction = new QAction(this);
    switchAction->setIcon(QIcon(":/icons/SWITCHip.png"));
    QToolButton *switchButton = new QToolButton();
    switchButton->setDefaultAction(switchAction);
    switchButton->setToolTip("SWITCH");
    switchButton->setFixedSize(96, 64);
    toolbar->addWidget(switchButton);
        
    QLabel *outputlabel = new QLabel("OUTPUT");
    outputlabel->setAlignment(Qt :: AlignCenter);
    toolbar->addWidget(outputlabel);
    outputlabel->setFont(font);
    outputlabel->setStyleSheet("background-color: lightgray;");

    QAction *ledAction = new QAction(this);
    ledAction->setIcon(QIcon(":/icons/LEDop.png"));
    QToolButton *ledButton = new QToolButton();
    ledButton->setDefaultAction(ledAction);
    ledButton->setToolTip("LED");
    ledButton->setFixedSize(96, 64);
    toolbar->addWidget(ledButton);


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

    /*connect(pointAction, &QAction::triggered, this, [this]() {
        Point *point = new Point();
        point->setPos(300, 200);
        scene->addItem(point);
    });*/

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