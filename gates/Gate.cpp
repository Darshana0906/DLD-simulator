#include "Gate.h"
#include "../CircuitScene.h"
#include <QPainter>

Gate::Gate(QGraphicsItem *parent): QGraphicsItem(parent) {
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
}

QRectF Gate::boundingRect() const {
    return QRectF(-32, -5, 164, 70);
}

void Gate::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(Qt::black);
    painter->setBrush(Qt::white);
    painter->drawPath(getPath());
}

void Gate::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // During placement mode, pass the click through to the scene
    // so a new element can be placed (instead of starting a drag).
    CircuitScene *cs = dynamic_cast<CircuitScene *>(scene());
    if (cs && cs->isPlacementMode()) {
        event->ignore();
        return;
    }
    QGraphicsItem::mousePressEvent(event);
}