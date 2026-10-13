#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "CircuitScene.h"
#include "ZoomGraphicsView.h"
#include "logic/CircuitAdapter.h"
#include "elements/Const1.h"
#include "elements/GND.h"
#include "elements/LED.h"
#include "elements/Point.h"
#include "elements/Switch.h"
#include "elements/Wire.h"
#include "gates/AndGate.h"
#include "gates/NandGate.h"
#include "gates/NorGate.h"
#include "gates/NotGate.h"
#include "gates/OrGate.h"
#include "gates/XnorGate.h"
#include "gates/XorGate.h"
#include "overlapping.h"
#include <QAction>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSize>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  scene = new CircuitScene(this);
  scene->setSceneRect(0, 0, 4000, 3000);

  ui->graphicsView->setScene(scene);
  ui->graphicsView->setCircuitScene(scene);
  ui->graphicsView->setRenderHint(QPainter::Antialiasing);
  ui->graphicsView->setViewportUpdateMode(
      QGraphicsView::BoundingRectViewportUpdate);
  ui->graphicsView->setCacheMode(QGraphicsView::CacheBackground);

  // Ensure graphicsView auto-resizes to fill available space
  if (!ui->centralwidget->layout()) {
    QVBoxLayout *layout = new QVBoxLayout(ui->centralwidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(ui->graphicsView);
  }

  // Connect CircuitScene signals for click-to-place
  connect(scene, &CircuitScene::sceneClicked, this,
          &MainWindow::onSceneClicked);
  connect(scene, &CircuitScene::placementCancelled, this,
          &MainWindow::cancelPlacement);
  connect(scene, &CircuitScene::errorMessage, this, [this](const QString &msg) {
    statusBar()->showMessage(msg, 3000);
    QMessageBox::warning(this, "Overlap Warning", msg);
  });

  // Fullscreen shortcut (F11)
  QAction *fullScreenAction = new QAction(this);
  fullScreenAction->setShortcut(QKeySequence(Qt::Key_F11));
  connect(fullScreenAction, &QAction::triggered, this,
          &MainWindow::toggleFullScreenMode);
  addAction(fullScreenAction);

  QToolBar *toolbar = new QToolBar("Toolbar", this);
  addToolBar(Qt::LeftToolBarArea, toolbar);
  toolbar->setIconSize(QSize(96, 64));

  // gates
  QFont font;
  font.setPointSize(12);

  QLabel *gatelabel = new QLabel("GATES");
  gatelabel->setAlignment(Qt ::AlignCenter);
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

  // QAction *pointAction = toolbar->addAction("Point");
  QLabel *connectionlabel = new QLabel("CONNECTION");
  connectionlabel->setAlignment(Qt ::AlignCenter);
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
  inputlabel->setAlignment(Qt ::AlignCenter);
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
  outputlabel->setAlignment(Qt ::AlignCenter);
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

  // Simulation Section in left toolbar
  QLabel *simlabel = new QLabel("SIMULATION");
  simlabel->setAlignment(Qt::AlignCenter);
  toolbar->addWidget(simlabel);
  simlabel->setFont(font);
  simlabel->setStyleSheet("background-color: lightgray;");

  QPushButton *simButton = new QPushButton("▶ SIMULATE");
  simButton->setToolTip("Simulate Circuit using Topological Sort (F5)");
  simButton->setFixedSize(96, 42);
  simButton->setStyleSheet(
      "QPushButton { background-color: #2E7D32; color: white; font-weight: bold; font-size: 11px; border-radius: 4px; border: 1px solid #1B5E20; }"
      "QPushButton:hover { background-color: #388E3C; }"
      "QPushButton:pressed { background-color: #1B5E20; }"
  );
  toolbar->addWidget(simButton);
  connect(simButton, &QPushButton::clicked, this, &MainWindow::runSimulation);

  // Top Controls Toolbar
  QToolBar *topControlBar = new QToolBar("Circuit Controls", this);
  topControlBar->setMovable(false);
  addToolBar(Qt::TopToolBarArea, topControlBar);

  QAction *simulateAction = new QAction("▶  Simulate Circuit", this);
  simulateAction->setShortcut(QKeySequence(Qt::Key_F5));
  simulateAction->setToolTip("Evaluate circuit states using Topological Sort (F5)");
  topControlBar->addAction(simulateAction);
  connect(simulateAction, &QAction::triggered, this, &MainWindow::runSimulation);

  topControlBar->addSeparator();

  QAction *clearAction = new QAction("🗑  Clear Circuit", this);
  clearAction->setToolTip("Remove all components and wires from canvas");
  topControlBar->addAction(clearAction);
  connect(clearAction, &QAction::triggered, this, &MainWindow::clearCircuit);

  // connections
  connect(andAction, &QAction::triggered, this,
          [this]() { setPlacementTool(ElementType::AndGate, "AND Gate"); });

  connect(orAction, &QAction::triggered, this,
          [this]() { setPlacementTool(ElementType::OrGate, "OR Gate"); });

  connect(notAction, &QAction::triggered, this,
          [this]() { setPlacementTool(ElementType::NotGate, "NOT Gate"); });

  connect(nandAction, &QAction::triggered, this,
          [this]() { setPlacementTool(ElementType::NandGate, "NAND Gate"); });

  connect(norAction, &QAction::triggered, this,
          [this]() { setPlacementTool(ElementType::NorGate, "NOR Gate"); });

  connect(xorAction, &QAction::triggered, this,
          [this]() { setPlacementTool(ElementType::XorGate, "XOR Gate"); });

  connect(xnorAction, &QAction::triggered, this,
          [this]() { setPlacementTool(ElementType::XnorGate, "XNOR Gate"); });

  connect(wireAction, &QAction::triggered, this,
          [this]() { setPlacementTool(ElementType::Wire, "Wire"); });

  connect(ledAction, &QAction::triggered, this,
          [this]() { setPlacementTool(ElementType::LED, "LED"); });

  connect(gndAction, &QAction::triggered, this,
          [this]() { setPlacementTool(ElementType::GND, "GND"); });

  connect(constantAction, &QAction::triggered, this,
          [this]() { setPlacementTool(ElementType::Const1, "Constant 1"); });

  connect(switchAction, &QAction::triggered, this,
          [this]() { setPlacementTool(ElementType::Switch, "Switch"); });
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setPlacementTool(ElementType type,
                                  const QString &elementName) {
  currentTool = type;
  if (type == ElementType::Wire) {
    scene->setWireToolActive(true);
    scene->setPlacementMode(false);
    statusBar()->showMessage("Wire Tool: Click a start pin, then click a "
                             "target pin (Right-click or Esc to cancel)");
  } else {
    scene->setWireToolActive(false);
    scene->setPlacementMode(true);
    statusBar()->showMessage(
        QString(
            "Click on the canvas to place %1 (Right-click or Esc to cancel)")
            .arg(elementName));
  }
  ui->graphicsView->applyCursor();
}

void MainWindow::onSceneClicked(const QPointF &pos) {
  if (currentTool != ElementType::None && currentTool != ElementType::Wire) {
    if (placeElement(currentTool, pos)) {
      cancelPlacement();
    } else {
      statusBar()->showMessage(
          "Cannot place here: Overlaps an existing component!", 3000);
      QMessageBox::warning(
          this, "Placement Overlap Warning",
          "Cannot place here: Overlaps an existing gate or wire!");
    }
  }
}

void MainWindow::cancelPlacement() {
  currentTool = ElementType::None;
  scene->setWireToolActive(false);
  scene->setPlacementMode(false);
  ui->graphicsView->applyCursor();
  statusBar()->clearMessage();
}

void MainWindow::toggleFullScreenMode() {
  if (isFullScreen()) {
    showMaximized();
  } else {
    showFullScreen();
  }
}

bool MainWindow::placeElement(ElementType type, const QPointF &pos) {
  if (type == ElementType::Wire) {
    Point *start = new Point();
    Point *end = new Point();
    start->setPos(pos.x() - 50, pos.y());
    end->setPos(pos.x() + 50, pos.y());
    Wire *wire = new Wire(start, end);
    scene->addItem(wire);
    return true;
  }

  QGraphicsItem *item = nullptr;
  QPointF itemPos;

  switch (type) {
  case ElementType::AndGate:
    item = new AndGate();
    itemPos = QPointF(pos.x() - 50, pos.y() - 30);
    break;
  case ElementType::OrGate:
    item = new OrGate();
    itemPos = QPointF(pos.x() - 50, pos.y() - 30);
    break;
  case ElementType::NotGate:
    item = new NotGate();
    itemPos = QPointF(pos.x() - 40, pos.y() - 30);
    break;
  case ElementType::NandGate:
    item = new NandGate();
    itemPos = QPointF(pos.x() - 50, pos.y() - 30);
    break;
  case ElementType::NorGate:
    item = new NorGate();
    itemPos = QPointF(pos.x() - 50, pos.y() - 30);
    break;
  case ElementType::XorGate:
    item = new XorGate();
    itemPos = QPointF(pos.x() - 50, pos.y() - 30);
    break;
  case ElementType::XnorGate:
    item = new XnorGate();
    itemPos = QPointF(pos.x() - 50, pos.y() - 30);
    break;
  case ElementType::LED:
    item = new LED();
    itemPos = QPointF(pos.x() - 25, pos.y() - 25);
    break;
  case ElementType::GND:
    item = new GND();
    itemPos = QPointF(pos.x() - 30, pos.y() - 25);
    break;
  case ElementType::Const1:
    item = new Const1();
    itemPos = QPointF(pos.x() - 30, pos.y() - 25);
    break;
  case ElementType::Switch:
    item = new Switch();
    itemPos = QPointF(pos.x() - 30, pos.y() - 25);
    break;
  default:
    return false;
  }

  if (!item)
    return false;

  item->setPos(itemPos);
  QRectF candidateSceneRect = item->sceneBoundingRect();

  // Check if candidate position collides with any existing component
  if (!canPlaceItem(scene, candidateSceneRect)) {
    delete item; // Discard candidate
    return false;
  }

  scene->addItem(item);
  if (scene->getAdapter()) {
    scene->getAdapter()->registerItem(item);
  }
  return true;
}

void MainWindow::runSimulation() {
  if (!scene || !scene->getAdapter())
    return;

  CircuitAdapter *adapter = scene->getAdapter();

  // 1. Sync all components on canvas and extract directed edges from wires
  adapter->syncSceneItems();
  adapter->extractWires();

  if (adapter->getNodeCount() == 0) {
    statusBar()->showMessage("Cannot simulate: Canvas is empty. Place components and wires first.", 3500);
    return;
  }

  // 2. Perform stepSimulation (topological evaluation via DAG)
  adapter->stepSimulation();

  // 3. User feedback
  if (adapter->hasCycle()) {
    statusBar()->showMessage("Simulation Warning: Combinational loop / cycle detected! Iterative relaxation applied.", 4000);
    QMessageBox::warning(this, "Cycle Detected",
                         "The circuit contains feedback loop(s) or cyclic dependencies.\n"
                         "Iterative relaxation was applied to evaluate steady state.");
  } else {
    auto order = adapter->getTopologicalOrder();
    statusBar()->showMessage(
        QString("Simulation successful! Evaluated %1 nodes in topological order.").arg(order.size()),
        4000
    );
  }
}

void MainWindow::clearCircuit() {
  if (QMessageBox::question(this, "Clear Circuit", "Are you sure you want to clear the entire circuit?") == QMessageBox::Yes) {
    scene->clear();
    if (scene->getAdapter()) {
      scene->getAdapter()->extractWires();
    }
    statusBar()->showMessage("Circuit cleared.", 2500);
  }
}

