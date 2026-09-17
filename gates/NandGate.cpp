#include "NandGate.h"

NandGate::NandGate(QGraphicsItem *parent)
    : Gate(parent)
{
}

QPainterPath NandGate::getPath() const
{
    QPainterPath path;

    path.moveTo(0, 0);
    path.lineTo(50, 0);

    path.cubicTo(85, 0, 100, 15, 100, 30);
    path.cubicTo(100, 45, 85, 60, 50, 60);

    path.lineTo(0, 60);
    path.closeSubpath();

    path.addEllipse(100, 25, 10, 10);

    return path;
}