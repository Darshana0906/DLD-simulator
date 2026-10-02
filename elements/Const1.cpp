#include "Const1.h"
#include "Point.h"
#include <QPainter>
#include <QPainterPath>
#include <QPen>

Const1::Const1(QGraphicsItem *parent) : Input(true, parent) {
    if (outputPoint) {
        outputPoint->setPos(50, 20);
    }
}

QRectF Const1::boundingRect() const {
    return QRectF(0, 0, 65, 50);
}

void Const1::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
    painter->setPen(pen);

    // Horizontal bottom bar connected to output pin (pin center at 55, 25)
    painter->drawLine(6, 25, 55, 25);

    // Vertical stem connecting bottom bar to the digit '1'
    painter->drawLine(30.5, 10, 30.5, 25);

    // digit '1' matching const1ip.png
    painter->drawLine(25.5, 8, 35.5, 8);
    painter->drawLine(30.5, 8, 30.5, -3);
    painter->drawLine(30.5, -3, 25.5, 2.5);


    painter->setBrush(Qt::black);
    //painter->drawPath(path);
}
