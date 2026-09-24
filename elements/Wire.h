#ifndef WIRE_H
#define WIRE_H

#include <QGraphicsItem>
#include <QPainterPath>
#include <QVector>
#include <QPointF>

class Point;

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

protected:
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    void calculateManhattanPath();

    Point *startPoint = nullptr;
    Point *endPoint = nullptr;
    QPointF m_previewPos;
    bool m_isPreview = false;

    QVector<QPointF> m_pathPoints;
    QRectF m_cachedBoundingRect;
};

#endif // WIRE_H