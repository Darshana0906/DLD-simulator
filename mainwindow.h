#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QPointF>
#include <QToolButton>

class CircuitScene;
class ReverseEquationDialog;
class UndoManager;
class EquationDialog;
class QAction;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

enum class ElementType {
  None,
  AndGate,
  OrGate,
  NotGate,
  NandGate,
  NorGate,
  XorGate,
  XnorGate,
  Wire,
  GND,
  Const1,
  Switch,
  LED
};

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void onSceneClicked(const QPointF &pos);
  void cancelPlacement();
  void toggleFullScreenMode();
  void runSimulation();
  void clearCircuit();
  void newCircuit();
  void newWindow();
  void openCircuit();
  void saveCircuit();
  void saveCircuitAs();
  void undo();
  void redo();
  void showLogicalEquation();
  void showReverseEquationDialog();
  void showTruthTable();

private:
  void setPlacementTool(ElementType type, const QString &elementName);
  bool placeElement(ElementType type, const QPointF &pos);
  void updateToolButtonHighlights(ElementType activeType);
  void updateWindowTitle();
  bool confirmDiscardChanges();

    Ui::MainWindow *ui;
    CircuitScene *scene;
    ElementType currentTool = ElementType::None;
    QMap<ElementType, class QToolButton*> toolButtons;
    QString currentFilePath;
    class EquationDialog *m_equationDialog = nullptr;
    class TruthTableDialog *m_truthTableDialog = nullptr;
    class QAction *m_selectAction = nullptr;
  Ui::MainWindow *ui;
  CircuitScene *scene;
  ElementType currentTool = ElementType::None;
  QMap<ElementType, QToolButton *> toolButtons;
  QString currentFilePath;
  UndoManager *undoManager = nullptr;
  QAction *undoAction = nullptr;
  QAction *redoAction = nullptr;
  class EquationDialog *m_equationDialog = nullptr;
  ReverseEquationDialog *m_reverseEquationDialog = nullptr;
  class QAction *m_selectAction = nullptr;
};

#endif // MAINWINDOW_H
