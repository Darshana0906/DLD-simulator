#ifndef CIRCUITSCENE_H
#define CIRCUITSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

class SelectionBox;
class GroupSelectionBox;
class Point;
class Wire;
class CircuitAdapter;

class CircuitScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit CircuitScene(QObject *parent = nullptr);
    ~CircuitScene();

    CircuitAdapter* getAdapter() const { return m_adapter; }

    void setPlacementMode(bool active);
    bool isPlacementMode() const;

    void setWireToolActive(bool active);
    bool isWireToolActive() const { return m_wireToolActive; }

    // Delete a single item (and its attached wires) cleanly
    void deleteItem(QGraphicsItem *item);
    void clearCircuit();

    void onPointClicked(Point *point);
    void clearWirePlacement();
    Point* findPinNear(const QPointF &scenePos, qreal maxDistance = 20.0);

signals:
    void sceneClicked(const QPointF &pos);
    void placementCancelled();
    void errorMessage(const QString &msg);

private slots:
    void onSelectionChanged();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    void clearSelectionBox();
    void clearGroupSelectionBox();
    void updateGroupSelectionBox();

    bool m_placementMode = false;
    bool m_wireToolActive = false;
    SelectionBox *m_activeSelectionBox = nullptr;
    GroupSelectionBox *m_groupSelectionBox = nullptr;

    Point *m_wireStartPoint = nullptr;
    Wire *m_draftWire = nullptr;
    Point *m_hoveredPin = nullptr;
    CircuitAdapter *m_adapter = nullptr;
};

#endif // CIRCUITSCENE_H
