#include "NorGate.h"
#include <QPainter>

NorGate::NorGate(QGraphicsItem *parent) : Gate(parent) {
    setInputCount(2);
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

    for (Point *pin : getInputPins()) {
        qreal y = pin->y() + 5;
        painter->drawLine(-20, y, 25, y);
    }
    painter->setBrush(Qt::white);
    painter->drawPath(getPath());
    
    painter->drawEllipse(100, 25, 10, 10);
    painter->drawLine(110, 30, 120, 30);
}