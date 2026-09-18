#include "OrGate.h"
#include <QPainter>

OrGate::OrGate(QGraphicsItem *parent)
    :Gate(parent) {
}

QPainterPath OrGate::getPath() const {
    QPainterPath path;
    path.moveTo(0, 0);
    path.cubicTo(30, 5, 45, 10, 100, 30);
    path.cubicTo(45, 50, 30, 55, 0, 60);
    path.cubicTo(20, 45, 25, 15, 0, 0);
    path.closeSubpath();
    return path;
}
void OrGate::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *,
                   QWidget *) {
    painter->setPen(Qt::black);
    painter->setBrush(Qt::white);
    painter->drawPath(getPath());
    painter->drawLine(-20, 15, 14, 15);
    painter->drawLine(-20, 45, 13, 45);
    painter->drawLine(100, 30, 120, 30);
    painter->drawEllipse(-29, 11, 8, 8);
    painter->drawEllipse(-29, 41, 8, 8);
    painter->drawEllipse(120, 26, 8, 8);
}