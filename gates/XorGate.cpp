#include "XorGate.h"
#include <QPainter>

XorGate::XorGate(QGraphicsItem *parent) : Gate(parent) {
    addInputPin(-29, 11);
    addInputPin(-29, 41);
    setOutputPin(120, 26);
}

QPainterPath XorGate::getPath() const {
    QPainterPath path;
    path.moveTo(0, 0);
    path.cubicTo(30, 5, 45, 10, 100, 30);
    path.cubicTo(45, 50, 30, 55, 0, 60);
    path.cubicTo(20, 45, 25, 15, 0, 0);
    path.closeSubpath();
    path.moveTo(-10, 0);
    path.cubicTo(10, 15, 10, 45, -10, 60);
    return path;
}

void XorGate::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(Qt::black);
    painter->setBrush(Qt::white);
    painter->drawPath(getPath());
    QPainterPath extraCurve;
    extraCurve.moveTo(-8, 0);
    extraCurve.cubicTo(12, 15, 12, 45, -8, 60);
    painter->drawPath(extraCurve);
    painter->drawLine(-20, 15, 4, 15);
    painter->drawLine(-20, 45, 4, 45);
    painter->drawLine(100, 30, 120, 30);
}