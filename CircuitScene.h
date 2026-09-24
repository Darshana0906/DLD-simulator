#ifndef CIRCUITSCENE_H
#define CIRCUITSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

class SelectionBox;
class Point;
class Wire;

class CircuitScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit CircuitScene(QObject *parent = nullptr);
    ~CircuitScene();

    void setPlacementMode(bool active);
    bool isPlacementMode() const;

    void setWireToolActive(bool active);
    bool isWireToolActive() const { return m_wireToolActive; }

    void deleteItem(QGraphicsItem *item);

    void onPointClicked(Point *point);
    void clearWirePlacement();
    Point* findPinNear(const QPointF &scenePos, qreal maxDistance = 20.0);

signals:
    void sceneClicked(const QPointF &pos);
    void placementCancelled();

private slots:
    void onSelectionChanged();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    void clearSelectionBox();

    bool m_placementMode = false;
    bool m_wireToolActive = false;
    SelectionBox *m_activeSelectionBox = nullptr;

    Point *m_wireStartPoint = nullptr;
    Wire *m_draftWire = nullptr;
    Point *m_hoveredPin = nullptr;
};

#endif // CIRCUITSCENE_H
