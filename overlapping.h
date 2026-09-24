#ifndef OVERLAPPING_H
#define OVERLAPPING_H

#include <QGraphicsItem>
#include <QRectF>

class QGraphicsScene;

bool canMoveItem(QGraphicsItem *movingItem, const QRectF &newRect);
bool canPlaceItem(QGraphicsItem *candidate, const QRectF &newRect, QGraphicsScene *scene);

#endif
