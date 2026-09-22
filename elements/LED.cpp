#include "LED.h"
#include "Point.h"
#include "../CircuitScene.h"

#include <QPainter>

LED::LED(QGraphicsItem *parent) : QGraphicsItem(parent) {
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);

    inputPoint = new Point(this, false);
    inputPoint->setPos(-10, 20);
}

Point* LED::getInputPoint() const {
    return inputPoint;
}

QRectF LED::boundingRect() const {
    return QRectF(-15, 0, 65, 50);
}

void LED::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    bool state = false;
    painter->setPen(Qt::black);
    if(state) 
        painter->setBrush(Qt::red);
    else 
        painter->setBrush(Qt::white);
    painter->drawEllipse(10, 10, 30, 30);
    painter->drawLine(0, 25, 10, 25);
}

void LED::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    CircuitScene *cs = dynamic_cast<CircuitScene *>(scene());
    if (cs && cs->isPlacementMode()) {
        event->ignore();
        return;
    }
    QGraphicsItem::mousePressEvent(event);
}