#include "Point.h"
#include <QPainter>
#include <QCursor>
Point::Point(QGraphicsItem *parent, bool movable) : QGraphicsItem(parent) {
    if (movable)
        setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
}

QRectF Point::boundingRect() const {
    return QRectF(0, 0, 10, 10);
}

void Point::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setBrush(Qt::black);
    painter->setPen(Qt::black);
    painter->drawEllipse(0, 0, 10, 10);
}
void Point::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    setCursor(QCursor(Qt::OpenHandCursor));
    QGraphicsItem::hoverEnterEvent(event);
}

void Point::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    unsetCursor();
    QGraphicsItem::hoverLeaveEvent(event);
}