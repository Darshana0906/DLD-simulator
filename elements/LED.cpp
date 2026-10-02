#include "LED.h"
#include "Point.h"
#include "Wire.h"
#include "../CircuitScene.h"
#include "../overlapping.h"

#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QColor>
#include <QRectF>
#include <QPointF>

LED::LED(QGraphicsItem *parent) : QGraphicsItem(parent), state(true) {
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    inputPoint = new Point(this, false, PinType::Input);
    inputPoint->setPos(-10, 20);
}

Point* LED::getInputPoint() const {
    return inputPoint;
}

bool LED::getState() const {
    return state;
}

void LED::setState(bool s) {
    if (state != s) {
        state = s;
        update();
    }
}

bool LED::getValue() const {
    return getState();
}

void LED::setValue(bool v) {
    setState(v);
}

bool LED::getOp() const {
    return getState();
}

void LED::setOp(bool op) {
    setState(op);
}

QRectF LED::boundingRect() const {
    return QRectF(-15, 0, 65, 50);
}

void LED::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setRenderHint(QPainter::Antialiasing);

    // Glass bulb body path matching ledop.png
    QPainterPath bulbPath;
    bulbPath.moveTo(20.5, 33.5);
    bulbPath.cubicTo(20.0, 27.0, 14.0, 23.0, 14.0, 17.0);
    bulbPath.cubicTo(14.0, 9.3, 20.3, 3.0, 28.0, 3.0);
    bulbPath.cubicTo(35.7, 3.0, 42.0, 9.3, 42.0, 17.0);
    bulbPath.cubicTo(42.0, 23.0, 36.0, 27.0, 35.5, 33.5);
    bulbPath.closeSubpath();

    // Bulb glows yellow when op is 1, white when 0
    painter->setBrush(state ? QColor(255, 235, 59) : Qt::white);
    painter->setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawPath(bulbPath);

    // Socket collar inside bulb
    painter->setBrush(state ? QColor(255, 245, 150) : Qt::white);
    painter->setPen(QPen(Qt::black, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
    painter->drawRect(QRectF(25.5, 29.5, 5.0, 4.0));

    // Filament supports
    painter->setPen(QPen(Qt::black, 1.5, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(QPointF(26.5, 29.5), QPointF(23.0, 20.0));
    painter->drawLine(QPointF(29.5, 29.5), QPointF(33.0, 20.0));

    // Zigzag filament
    QPainterPath filament;
    filament.moveTo(23.0, 20.0);
    filament.lineTo(24.7, 18.2);
    filament.lineTo(26.3, 20.0);
    filament.lineTo(28.0, 18.2);
    filament.lineTo(29.7, 20.0);
    filament.lineTo(31.3, 18.2);
    filament.lineTo(33.0, 20.0);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(filament);

    // Screw base threads (3 rounded ribs)
    painter->setBrush(Qt::white);
    painter->setPen(QPen(Qt::black, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawRoundedRect(QRectF(21.8, 34.0, 12.4, 3.0), 1.5, 1.5);
    painter->drawRoundedRect(QRectF(22.0, 37.2, 12.0, 3.0), 1.5, 1.5);
    painter->drawRoundedRect(QRectF(22.5, 40.4, 11.0, 3.0), 1.5, 1.5);

    // Bottom base contact terminal
    QPainterPath baseTip;
    baseTip.moveTo(24.0, 43.4);
    baseTip.quadTo(28.0, 47.0, 32.0, 43.4);
    baseTip.closeSubpath();
    painter->setBrush(QColor(50, 50, 50));
    painter->setPen(QPen(Qt::black, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawPath(baseTip);

    // Terminal lead wire connected to input pin (pin center is at -5, 25)
    painter->setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(QPointF(-5, 25), QPointF(17.5, 25));
}

void LED::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    CircuitScene *cs = dynamic_cast<CircuitScene *>(scene());
    if (cs && cs->isPlacementMode()) {
        event->ignore();
        return;
    }
    QGraphicsItem::mousePressEvent(event);
}

QVariant LED::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == QGraphicsItem::ItemPositionChange && scene()) {
        const QPointF delta = parentItem()
            ? parentItem()->mapToScene(value.toPointF()) - parentItem()->mapToScene(pos())
            : value.toPointF() - pos();
        if (!canMoveItem(this, sceneBoundingRect().translated(delta)))
            return pos();
    }
    return QGraphicsItem::itemChange(change, value);
}
