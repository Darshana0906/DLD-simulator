#ifndef OVERLAPPING_H
#define OVERLAPPING_H

#include <QGraphicsItem>
#include <QRectF>

bool canMoveItem(QGraphicsItem *movingItem, const QRectF &newRect);

#endif