#include "NandGate.h"
#include <QPainter>

NandGate::NandGate(QGraphicsItem *parent) : Gate(parent) {
    setInputCount(2);
    setOutputPin(120, 26);
}

QPainterPath NandGate::getPath() const {
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

void NandGate::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(Qt::black);
    for (Point *pin : getInputPins()) {
        qreal y = pin->y() + 5;
        painter->drawLine(-20, y, 25, y);
    }
    painter->setBrush(Qt::white);
    painter->drawPath(getPath());

    painter->drawEllipse(100, 25, 10, 10);

    painter->drawLine(110, 30, 120, 30);
}