#include "Switch.h"

Switch::Switch(QGraphicsItem *parent) : Input(false, parent) {}

void Switch::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    setValue(!getValue());
    QGraphicsItem::mousePressEvent(event);
}