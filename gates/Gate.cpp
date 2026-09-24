#include "Gate.h"
#include "../CircuitScene.h"
#include "../elements/Wire.h"
#include <QPainter>

Gate::Gate(QGraphicsItem *parent): QGraphicsItem(parent) {
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

void Gate::addInputPin(qreal x, qreal y) {
    Point *pin = new Point(this, false, PinType::Input);
    pin->setPos(x, y);
    m_inputPins.append(pin);
}

void Gate::setOutputPin(qreal x, qreal y) {
    if (m_outputPin) {
        delete m_outputPin;
    }
    m_outputPin = new Point(this, false, PinType::Output);
    m_outputPin->setPos(x, y);
}

QRectF Gate::boundingRect() const {
    return QRectF(-35, -5, 170, 70);
}

void Gate::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(Qt::black);
    painter->setBrush(Qt::white);
    painter->drawPath(getPath());
}

void Gate::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    CircuitScene *cs = dynamic_cast<CircuitScene *>(scene());
    if (cs && cs->isPlacementMode()) {
        event->ignore();
        return;
    }
    QGraphicsItem::mousePressEvent(event);
}

QVariant Gate::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionHasChanged || change == ItemTransformHasChanged) {
        // Update all connected wires on input pins
        for (Point *pin : m_inputPins) {
            for (Wire *wire : pin->getWires()) {
                wire->updatePath();
            }
        }
        // Update all connected wires on output pin
        if (m_outputPin) {
            for (Wire *wire : m_outputPin->getWires()) {
                wire->updatePath();
            }
        }
    }
    return QGraphicsItem::itemChange(change, value);
}