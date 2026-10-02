#include "NotGate.h"
#include <QPainter>

NotGate::NotGate(QGraphicsItem *parent) : Gate(parent) {
    addInputPin(-29, 26);
    setOutputPin(100, 26);
}

QPainterPath NotGate::getPath() const {
    QPainterPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 60);
    path.lineTo(80, 30);
    path.closeSubpath();
    path.addEllipse(80, 25, 10, 10);
    return path;
}

void NotGate::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(Qt::black);
    painter->setBrush(Qt::white);
    painter->drawPath(getPath());
    painter->drawLine(-20, 30, 0, 30);
    painter->drawLine(90, 30, 100, 30);
}