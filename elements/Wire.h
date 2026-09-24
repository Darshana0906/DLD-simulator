#ifndef WIRE_H
#define WIRE_H

#include <QGraphicsItem>

class Point;

class Wire : public QGraphicsItem {
public:
    Wire(Point *start, Point *end, QGraphicsItem *parent = nullptr);
protected:
    QRectF boundingRect() const override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
private:
    Point *startPoint;
    Point *endPoint;
};

#endif
