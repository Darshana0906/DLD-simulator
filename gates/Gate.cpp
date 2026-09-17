#include "Gate.h"
#include <QPainter>

Gate::Gate(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
}

QRectF Gate::boundingRect() const
{
    return QRectF(0, 0, 100, 60);
}

void Gate::paint(QPainter *painter,const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(Qt::black);
    painter->setBrush(Qt::white);
    painter->drawPath(getPath());
}