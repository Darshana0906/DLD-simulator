#include "GND.h"
#include "Point.h"
#include <QPainter>
#include <QPen>

GND::GND(QGraphicsItem *parent) : Input(false, parent) {
    if (outputPoint) {
        outputPoint->setPos(50, 10);
    }
}

QRectF GND::boundingRect() const {
    return QRectF(0, 0, 65, 50);
}

void GND::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
    painter->setPen(pen);

    // Terminal lead wire connected to output pin (pin center is at 55, 15)
    painter->drawLine(20, 15, 55, 15);

    // Vertical stem down to the top ground line
    painter->drawLine(20, 15, 20, 27);

    // Four horizontal lines of decreasing width
    painter->drawLine(4, 27, 36, 27);   // Top line (width 32)
    painter->drawLine(8, 33, 32, 33);   // Second line (width 24)
    painter->drawLine(12, 39, 28, 39);  // Third line (width 16)
    painter->drawLine(16, 45, 24, 45);  // Bottom line (width 8)
}