#include "NorGate.h"
#include <QPainter>

NorGate::NorGate(QGraphicsItem *parent) : Gate(parent) {
    addInputPin(-29, 11);
    addInputPin(-29, 41);
    setOutputPin(120, 26);
}

QPainterPath NorGate::getPath() const {
    QPainterPath path;
    path.moveTo(0, 0);
    path.cubicTo(30, 5, 45, 10, 100, 30);
    path.cubicTo(45, 50, 30, 55, 0, 60);
    path.cubicTo(20, 45, 25, 15, 0, 0);
    path.closeSubpath();
    path.addEllipse(100, 25, 10, 10);
    return path;
}

void NorGate::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(Qt::black);
    painter->setBrush(Qt::white);
    painter->drawPath(getPath());
    painter->drawLine(-20, 15, 14, 15);
    painter->drawLine(-20, 45, 13, 45);
    painter->drawEllipse(100, 25, 10, 10);
    painter->drawLine(110, 30, 120, 30);
}