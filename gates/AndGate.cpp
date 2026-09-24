#include "AndGate.h"
#include <QPainter>

AndGate::AndGate(QGraphicsItem *parent) : Gate(parent) {
    addInputPin(-29, 11);
    addInputPin(-29, 41);
    setOutputPin(120, 26);
}

QPainterPath AndGate::getPath() const {
    QPainterPath path;
    path.moveTo(0, 0);
    path.lineTo(50, 0);
    path.cubicTo(85, 0, 100, 15, 100, 30);
    path.cubicTo(100, 45, 85, 60, 50, 60);
    path.lineTo(0, 60);
    path.closeSubpath();
    return path;
}

void AndGate::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(Qt::black);
    painter->setBrush(Qt::white);
    painter->drawPath(getPath());
    painter->drawLine(-20, 15, 0, 15);
    painter->drawLine(-20, 45, 0, 45);
    painter->drawLine(100, 30, 120, 30);
}