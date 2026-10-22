#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointF>
#include <QMap>

class CircuitScene;
class UndoManager;
class QToolButton;
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
    void showLogicalEquation();

private:
    void setPlacementTool(ElementType type, const QString &elementName);
    bool placeElement(ElementType type, const QPointF &pos);
    void updateToolButtonHighlights(ElementType activeType);
    void updateWindowTitle();
    bool confirmDiscardChanges();

    Ui::MainWindow *ui;
    CircuitScene *scene;
    ElementType currentTool = ElementType::None;
    QMap<ElementType, QToolButton*> toolButtons;
    QString currentFilePath;
    class EquationDialog *m_equationDialog = nullptr;
    class QAction *m_selectAction = nullptr;
};

#endif // MAINWINDOW_H
