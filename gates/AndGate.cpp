#include "AndGate.h"
#include <QPainter>

AndGate::AndGate(QGraphicsItem *parent)
    : Gate(parent)
{
}

QPainterPath AndGate::getPath() const
{
    QPainterPath path;

    path.moveTo(0, 0);
    path.lineTo(50, 0);

    path.cubicTo(85, 0, 100, 15, 100, 30);
    path.cubicTo(100, 45, 85, 60, 50, 60);

    path.lineTo(0, 60);
    path.closeSubpath();

    return path;
}