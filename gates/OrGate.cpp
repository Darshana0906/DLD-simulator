#include "OrGate.h"
#include <QPainter>

OrGate::OrGate(QGraphicsItem *parent)
    : Gate(parent)
{
}

QPainterPath OrGate::getPath() const
{
    QPainterPath path;

    path.moveTo(0, 0);

    path.cubicTo(30, 5, 45, 10, 100, 30);

    path.cubicTo(45, 50, 30, 55, 0, 60);

    path.cubicTo(20, 45, 25, 15, 0, 0);

    path.closeSubpath();

    return path;
}