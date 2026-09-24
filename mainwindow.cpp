#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ZoomGraphicsView.h"
#include "CircuitScene.h"
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
#include "overlapping.h"
#include <QToolBar>
#include <QAction>
#include <QToolButton>
#include <QIcon>
#include <QSize>
#include <QLabel>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QKeySequence>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    scene = new CircuitScene(this);
    scene->setSceneRect(0, 0, 4000, 3000);

    ui->graphicsView->setScene(scene);
    ui->graphicsView->setCircuitScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    ui->graphicsView->setCacheMode(QGraphicsView::CacheBackground);

    // Ensure graphicsView auto-resizes to fill available space
    if (!ui->centralwidget->layout()) {
        QVBoxLayout *layout = new QVBoxLayout(ui->centralwidget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(ui->graphicsView);
    }

    // Connect CircuitScene signals for click-to-place
    connect(scene, &CircuitScene::sceneClicked, this, &MainWindow::onSceneClicked);
    connect(scene, &CircuitScene::placementCancelled, this, &MainWindow::cancelPlacement);

    // Fullscreen shortcut (F11)
    QAction *fullScreenAction = new QAction(this);
    fullScreenAction->setShortcut(QKeySequence(Qt::Key_F11));
    connect(fullScreenAction, &QAction::triggered, this, &MainWindow::toggleFullScreenMode);
    addAction(fullScreenAction);

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
        setPlacementTool(ElementType::AndGate, "AND Gate");
    });

    connect(orAction, &QAction::triggered, this, [this]() {
        setPlacementTool(ElementType::OrGate, "OR Gate");
    });

    connect(notAction, &QAction::triggered, this, [this]() {
        setPlacementTool(ElementType::NotGate, "NOT Gate");
    });

    connect(nandAction, &QAction::triggered, this, [this]() {
        setPlacementTool(ElementType::NandGate, "NAND Gate");
    });

    connect(norAction, &QAction::triggered, this, [this]() {
        setPlacementTool(ElementType::NorGate, "NOR Gate");
    });

    connect(xorAction, &QAction::triggered, this, [this]() {
        setPlacementTool(ElementType::XorGate, "XOR Gate");
    });

    connect(xnorAction, &QAction::triggered, this, [this]() {
        setPlacementTool(ElementType::XnorGate, "XNOR Gate");
    });

    connect(wireAction, &QAction::triggered, this, [this]() {
        setPlacementTool(ElementType::Wire, "Wire");
    });

    connect(ledAction, &QAction::triggered, this, [this]() {
        setPlacementTool(ElementType::LED, "LED");
    });

    connect(gndAction, &QAction::triggered, this, [this]() {
        setPlacementTool(ElementType::GND, "GND");
    });

    connect(constantAction, &QAction::triggered, this, [this]() {
        setPlacementTool(ElementType::Const1, "Constant 1");
    });

    connect(switchAction, &QAction::triggered, this, [this]() {
        setPlacementTool(ElementType::Switch, "Switch");
    });
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setPlacementTool(ElementType type, const QString &elementName) {
    currentTool = type;
    scene->setPlacementMode(true);
    ui->graphicsView->applyCursor();   // enforce CrossCursor via single path
    statusBar()->showMessage(QString("Click on the canvas to place %1 (Right-click or Esc to cancel)").arg(elementName));
}

void MainWindow::onSceneClicked(const QPointF &pos) {
    if (currentTool != ElementType::None) {
        placeElement(currentTool, pos);
        cancelPlacement();
    }
}

void MainWindow::cancelPlacement() {
    currentTool = ElementType::None;
    scene->setPlacementMode(false);
    ui->graphicsView->applyCursor();   // restore ArrowCursor via single path
    statusBar()->clearMessage();
}

void MainWindow::toggleFullScreenMode() {
    if (isFullScreen()) {
        showMaximized();
    } else {
        showFullScreen();
    }
}

void MainWindow::placeElement(ElementType type, const QPointF &pos) {
    auto addIfAllowed = [this](QGraphicsItem *item) {
        const QRectF proposedRect = item->mapRectToScene(item->boundingRect());
        if (canPlaceItem(item, proposedRect, scene)) {
            scene->addItem(item);
            return true;
        }
        delete item;
        return false;
    };

    switch (type) {
    case ElementType::AndGate: {
        AndGate *gate = new AndGate();
        gate->setPos(pos.x() - 50, pos.y() - 30);
        addIfAllowed(gate);
        break;
    }
    case ElementType::OrGate: {
        OrGate *gate = new OrGate();
        gate->setPos(pos.x() - 50, pos.y() - 30);
        addIfAllowed(gate);
        break;
    }
    case ElementType::NotGate: {
        NotGate *gate = new NotGate();
        gate->setPos(pos.x() - 40, pos.y() - 30);
        addIfAllowed(gate);
        break;
    }
    case ElementType::NandGate: {
        NandGate *gate = new NandGate();
        gate->setPos(pos.x() - 50, pos.y() - 30);
        addIfAllowed(gate);
        break;
    }
    case ElementType::NorGate: {
        NorGate *gate = new NorGate();
        gate->setPos(pos.x() - 50, pos.y() - 30);
        addIfAllowed(gate);
        break;
    }
    case ElementType::XorGate: {
        XorGate *gate = new XorGate();
        gate->setPos(pos.x() - 50, pos.y() - 30);
        addIfAllowed(gate);
        break;
    }
    case ElementType::XnorGate: {
        XnorGate *gate = new XnorGate();
        gate->setPos(pos.x() - 50, pos.y() - 30);
        addIfAllowed(gate);
        break;
    }
    case ElementType::Wire: {
        Point *start = new Point();
        Point *end = new Point();
        start->setPos(pos.x() - 50, pos.y());
        end->setPos(pos.x() + 50, pos.y());
        Wire *wire = new Wire(start, end);
        if (addIfAllowed(wire)) {
            scene->addItem(start);
            scene->addItem(end);
        } else {
            delete start;
            delete end;
        }
        break;
    }
    case ElementType::LED: {
        LED *led = new LED();
        led->setPos(pos.x() - 25, pos.y() - 25);
        addIfAllowed(led);
        break;
    }
    case ElementType::GND: {
        GND *gnd = new GND();
        gnd->setPos(pos.x() - 30, pos.y() - 25);
        addIfAllowed(gnd);
        break;
    }
    case ElementType::Const1: {
        Const1 *constant = new Const1();
        constant->setPos(pos.x() - 30, pos.y() - 25);
        addIfAllowed(constant);
        break;
    }
    case ElementType::Switch: {
        Switch *sw = new Switch();
        sw->setPos(pos.x() - 30, pos.y() - 25);
        addIfAllowed(sw);
        break;
    }
    default:
        break;
    }
}
