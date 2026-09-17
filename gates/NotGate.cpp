#include "NotGate.h"

NotGate::NotGate(QGraphicsItem *parent)
    : Gate(parent)
{
}

QPainterPath NotGate::getPath() const
{
    QPainterPath path;

    path.moveTo(0, 0);
    path.lineTo(0, 60);
    path.lineTo(80, 30);
    path.closeSubpath();

    path.addEllipse(80, 25, 10, 10);

    return path;
}