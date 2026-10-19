#ifndef WIRE_H
#define WIRE_H

#include <QGraphicsItem>
#include <QPainterPath>
#include <QVector>
#include <QPointF>
#include "Point.h"

class Wire : public QGraphicsItem {
public:
    Wire(Point *start, Point *end = nullptr, QGraphicsItem *parent = nullptr);
    ~Wire();

    Point* getStartPoint() const { return startPoint; }
    Point* getEndPoint() const { return endPoint; }
    void setEndPoint(Point *end);

    void setPreviewPos(const QPointF &pos);

    void updatePath();

    QVector<QPointF> getSegments() const { return m_pathPoints; }

    int getLogicValue() const { return m_logicValue; }
    void setLogicValue(int val) {
        if (m_logicValue != val) {
            m_logicValue = val;
            update();
        }
    }

    // Safely detach from a Point that is being destroyed.
    // Saves the last known position so the wire can still draw itself.
    void detachPoint(Point *point);

protected:
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    void calculateManhattanPath();

    Point *startPoint = nullptr;
    Point *endPoint = nullptr;
    QPointF m_previewPos;
    bool m_isPreview = false;

    QVector<QPointF> m_pathPoints;
    QRectF m_cachedBoundingRect;

    int m_logicValue = -1; // -1: Undefined, 0: Low, 1: High

    // Saved positions for endpoints that have been destroyed.
    // When a Point is deleted, the wire remembers where it was so it can
    // still draw itself in the correct location.
    QPointF m_savedStartPos;
    QPointF m_savedEndPos;
    PinType m_savedStartPinType = PinType::Standalone;
    PinType m_savedEndPinType = PinType::Input;
    bool m_hasStartPos = false;
    bool m_hasEndPos = false;
};

#endif
