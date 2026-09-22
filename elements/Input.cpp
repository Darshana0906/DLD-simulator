#include "Input.h"
#include "Point.h"
#include "../CircuitScene.h"

#include <QPainter>

Input::Input(bool value, QGraphicsItem *parent) : QGraphicsItem(parent), value(value) {
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    outputPoint = new Point(this, false);
    outputPoint->setPos(55, 20);
}

bool Input::getValue() const {
    return value;
}

void Input::setValue(bool value) {
    this->value = value;
    update();
}

Point* Input::getOutputPoint() const {
    return outputPoint;
}

QRectF Input::boundingRect() const {
    return QRectF(0, 0, 65, 50);
}

void Input::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(Qt::black);
    if (value)
        painter->setBrush(Qt::green);
    else
        painter->setBrush(Qt::darkGray);


    painter->drawRect(10, 10, 40, 30);
    painter->drawLine(50, 25, 60, 25);
}

void Input::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // During placement mode pass through so the scene can handle placement.
    CircuitScene *cs = dynamic_cast<CircuitScene *>(scene());
    if (cs && cs->isPlacementMode()) {
        event->ignore();
        return;
    }
    QGraphicsItem::mousePressEvent(event);
}