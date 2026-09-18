#include "Wire.h"
#include "Point.h"
#include <QPainter>

Wire::Wire(Point *start, Point *end, QGraphicsItem *parent) : QGraphicsItem(parent), startPoint(start), endPoint(end) {
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
}

QRectF Wire::boundingRect() const {
    if (!startPoint || !endPoint)
        return QRectF();

    QPointF start = startPoint->mapToScene(QPointF(5, 5));
    QPointF end = endPoint->mapToScene(QPointF(5, 5));

    return QRectF(start, end).normalized().adjusted(-5, -5, 5, 5);
}
void Wire::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    if (!startPoint || !endPoint)
        return;

    painter->setPen(QPen(Qt::black, 2)); 

    QPointF start = mapFromScene(startPoint->mapToScene(QPointF(5, 5)));

    QPointF end = mapFromScene(endPoint->mapToScene(QPointF(5, 5)));

    painter->drawLine(start, end);
}