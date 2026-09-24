#include "overlapping.h"
#include "elements/Point.h"
#include "elements/Wire.h"
#include "SelectionBox.h"
#include <QGraphicsScene>

static bool canOverlap(QGraphicsItem *candidate, const QRectF &newRect, QGraphicsScene *scene) {
    if (!candidate || dynamic_cast<Point *>(candidate))
        return true;
    if (!scene)
        return true;

    for (QGraphicsItem *item : scene->items()) {
        if (item == candidate)
            continue;
        if (dynamic_cast<SelectionBox *>(item))
            continue;
        if (dynamic_cast<Point *>(item))
            continue;
        if (dynamic_cast<Wire *>(candidate) && dynamic_cast<Wire *>(item))
            continue;
        if (newRect.intersects(item->sceneBoundingRect()))
            return false;
    }
    return true;
}

bool canMoveItem(QGraphicsItem *movingItem, const QRectF &newRect) {
    return canOverlap(movingItem, newRect, movingItem ? movingItem->scene() : nullptr);
}

bool canPlaceItem(QGraphicsItem *candidate, const QRectF &newRect, QGraphicsScene *scene) {
    return canOverlap(candidate, newRect, scene);
}
