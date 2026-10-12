#ifndef OVERLAPPING_H
#define OVERLAPPING_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QRectF>

class Wire;

// Checks whether an item is a solid component (Gate, Input, Switch, LED, GND,
// Const1)
bool isSolidComponent(QGraphicsItem *item);

// Checks if a candidate bounding rect in scene coordinates overlaps any
// existing component in the scene
bool canPlaceItem(QGraphicsScene *scene, const QRectF &sceneRect,
                  QGraphicsItem *ignoreItem = nullptr);

// Checks if moving or resizing an item to candidateSceneRect is collision-free
bool canMoveItem(QGraphicsItem *movingItem, const QRectF &candidateSceneRect);

// Checks if a candidate wire polyline path crosses over any solid components
// (other than the components it connects to)
bool canPlaceWire(Wire *wire);

#endif
