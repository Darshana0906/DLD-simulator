#include "overlapping.h"
#include "elements/Point.h"
#include "elements/Wire.h"
#include <QGraphicsScene>

bool canMoveItem(QGraphicsItem *movingItem, const QRectF &newRect){
    if (dynamic_cast<Point *>(movingItem))
        return true;
    if (!movingItem->scene())
        return true;

    for (QGraphicsItem *item : movingItem->scene()->items()) {
        if (item == movingItem)
            continue;
        if (dynamic_cast<Point *>(item))
            continue;
        if (dynamic_cast<Wire *>(movingItem) && dynamic_cast<Wire *>(item))
            continue;
        if (newRect.intersects(item->sceneBoundingRect()))
            return false;
    }
    return true;
}