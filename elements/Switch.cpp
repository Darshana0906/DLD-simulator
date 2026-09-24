#include "Switch.h"
#include "CircuitScene.h"
#include <QGraphicsScene>

Switch::Switch(QGraphicsItem *parent) : Input(false, parent) {}

void Switch::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // During placement mode the scene needs to receive this click to place
    // a new element — do not consume it or toggle state here.
    CircuitScene *cs = dynamic_cast<CircuitScene *>(scene());
    if (cs && cs->isPlacementMode()) {
        event->ignore(); // let the scene's mousePressEvent handle it
        return;
    }
    setValue(!getValue());
    QGraphicsItem::mousePressEvent(event);
}