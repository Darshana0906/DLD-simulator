#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointF>

class CircuitScene;

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

private:
    void setPlacementTool(ElementType type, const QString &elementName);
    void placeElement(ElementType type, const QPointF &pos);

    Ui::MainWindow *ui;
    CircuitScene *scene;
    ElementType currentTool = ElementType::None;
};

#endif // MAINWINDOW_H