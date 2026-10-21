#include "overlapping.h"
#include "SelectionBox.h"
#include "GroupSelectionBox.h"
#include "elements/Input.h"
#include "elements/LED.h"
#include "elements/Point.h"
#include "elements/Wire.h"
#include "gates/Gate.h"
#include <QGraphicsScene>

bool isSolidComponent(QGraphicsItem *item) {
  if (!item)
    return false;
  // Wires, Points/Pins, and SelectionBoxes do NOT block component
  // placement/movement
  if (dynamic_cast<Wire *>(item))
    return false;
  if (dynamic_cast<Point *>(item))
    return false;
  if (dynamic_cast<SelectionBox *>(item))
    return false;
  if (dynamic_cast<GroupSelectionBox *>(item))
    return false;

  // Gates and input/output components are solid components
  if (dynamic_cast<Gate *>(item))
    return true;
  if (dynamic_cast<Input *>(item))
    return true; // Covers Switch, GND, Const1
  if (dynamic_cast<LED *>(item))
    return true;

  return false;
}

bool canPlaceItem(QGraphicsScene *scene, const QRectF &sceneRect,
                  QGraphicsItem *ignoreItem) {
  if (!scene)
    return true;

  for (QGraphicsItem *item : scene->items()) {
    if (!item || item == ignoreItem)
      continue;

    // Skip child items of the ignored item (e.g. selection box or pins)
    if (ignoreItem && item->topLevelItem() == ignoreItem->topLevelItem())
      continue;

    if (isSolidComponent(item)) {
      if (sceneRect.intersects(item->sceneBoundingRect())) {
        return false; // Collision detected with solid component!
      }
    } else if (Wire *wire = dynamic_cast<Wire *>(item)) {
      // Wires cannot be overlapped by solid components
      QVector<QPointF> segments = wire->getSegments();
      for (int i = 0; i + 1 < segments.size(); ++i) {
        QPointF p1 = wire->mapToScene(segments[i]);
        QPointF p2 = wire->mapToScene(segments[i + 1]);
        QLineF seg(p1, p2);
        QPointF intersectionPoint;
        // Check if segment intersects any of the 4 edges of candidate sceneRect
        QLineF top(sceneRect.topLeft(), sceneRect.topRight());
        QLineF bottom(sceneRect.bottomLeft(), sceneRect.bottomRight());
        QLineF left(sceneRect.topLeft(), sceneRect.bottomLeft());
        QLineF right(sceneRect.topRight(), sceneRect.bottomRight());

        if (sceneRect.contains(p1) || sceneRect.contains(p2) ||
            seg.intersects(top, &intersectionPoint) ==
                QLineF::BoundedIntersection ||
            seg.intersects(bottom, &intersectionPoint) ==
                QLineF::BoundedIntersection ||
            seg.intersects(left, &intersectionPoint) ==
                QLineF::BoundedIntersection ||
            seg.intersects(right, &intersectionPoint) ==
                QLineF::BoundedIntersection) {
          return false;
        }
      }
    }
  }
  return true;
}

bool canMoveItem(QGraphicsItem *movingItem, const QRectF &candidateSceneRect) {
  if (!movingItem)
    return true;
  if (!movingItem->scene())
    return true;

  // Only solid components are restricted from overlapping each other and wires
  if (!isSolidComponent(movingItem))
    return true;

  QGraphicsItem *topItem = movingItem->topLevelItem();

  for (QGraphicsItem *item : movingItem->scene()->items()) {
    if (!item || item == movingItem || item->topLevelItem() == topItem)
      continue;

    // GROUP MOVE: both items are selected -> moving together, never collide
    QGraphicsItem *itemTop = item->topLevelItem() ? item->topLevelItem() : item;
    if (movingItem->isSelected() && (item->isSelected() || itemTop->isSelected()))
      continue;

    if (isSolidComponent(item)) {
      if (candidateSceneRect.intersects(item->sceneBoundingRect())) {
        return false;
      }
    } else if (Wire *wire = dynamic_cast<Wire *>(item)) {
      // Skip wires connected directly to movingItem
      if (wire->getStartPoint() &&
          wire->getStartPoint()->topLevelItem() == topItem)
        continue;
      if (wire->getEndPoint() && wire->getEndPoint()->topLevelItem() == topItem)
        continue;

      // Skip wires that are themselves selected (part of the moving group)
      if (wire->isSelected())
        continue;

      // Skip wires whose endpoints belong to co-selected components
      if (movingItem->isSelected()) {
        bool startSel = wire->getStartPoint() &&
                        wire->getStartPoint()->topLevelItem() &&
                        wire->getStartPoint()->topLevelItem()->isSelected();
        bool endSel   = wire->getEndPoint() &&
                        wire->getEndPoint()->topLevelItem() &&
                        wire->getEndPoint()->topLevelItem()->isSelected();
        if (startSel || endSel)
          continue;
      }

      QVector<QPointF> segments = wire->getSegments();
      for (int i = 0; i + 1 < segments.size(); ++i) {
        QPointF p1 = wire->mapToScene(segments[i]);
        QPointF p2 = wire->mapToScene(segments[i + 1]);
        QLineF seg(p1, p2);
        QPointF intersectionPoint;
        QLineF top(candidateSceneRect.topLeft(), candidateSceneRect.topRight());
        QLineF bottom(candidateSceneRect.bottomLeft(),
                      candidateSceneRect.bottomRight());
        QLineF left(candidateSceneRect.topLeft(),
                    candidateSceneRect.bottomLeft());
        QLineF right(candidateSceneRect.topRight(),
                     candidateSceneRect.bottomRight());

        if (candidateSceneRect.contains(p1) ||
            candidateSceneRect.contains(p2) ||
            seg.intersects(top, &intersectionPoint) ==
                QLineF::BoundedIntersection ||
            seg.intersects(bottom, &intersectionPoint) ==
                QLineF::BoundedIntersection ||
            seg.intersects(left, &intersectionPoint) ==
                QLineF::BoundedIntersection ||
            seg.intersects(right, &intersectionPoint) ==
                QLineF::BoundedIntersection) {
          return false;
        }
      }
    }
  }
  return true;
}

#include "gates/NotGate.h"
#include <QSet>

QRectF getComponentBodyRect(QGraphicsItem *item) {
  if (!item)
    return QRectF();

  QGraphicsItem *top = item->topLevelItem();
  if (!top)
    return QRectF();

  if (Gate *gate = dynamic_cast<Gate *>(top)) {
    // Gate body: excludes terminal pins.
    // Input pins are at x <= -24, output pin is at x >= 120 (x >= 100 for NotGate).
    // The gate body spans x from -15 to 110 (90 for NotGate), y from -2 to 62.
    qreal width = dynamic_cast<NotGate *>(top) ? 105.0 : 125.0;
    QRectF bodyLocal(-15.0, -2.0, width, 64.0);
    return gate->mapToScene(bodyLocal).boundingRect();
  }
  if (Input *input = dynamic_cast<Input *>(top)) {
    // Input box from (8, 8, 44, 34)
    QRectF bodyLocal(8.0, 8.0, 44.0, 34.0);
    return input->mapToScene(bodyLocal).boundingRect();
  }
  if (LED *led = dynamic_cast<LED *>(top)) {
    // LED bulb body from (10, 0, 36, 50)
    QRectF bodyLocal(10.0, 0.0, 36.0, 50.0);
    return led->mapToScene(bodyLocal).boundingRect();
  }

  return top->sceneBoundingRect();
}

bool canPlaceWire(Wire *wire) {
  if (!wire || !wire->scene())
    return true;

  QVector<QPointF> segments = wire->getSegments();
  if (segments.size() < 2)
    return true;

  QSet<QGraphicsItem *> checkedTops;

  for (QGraphicsItem *item : wire->scene()->items()) {
    if (!item || item == wire)
      continue;
    if (!isSolidComponent(item))
      continue;

    QGraphicsItem *solidTop = item->topLevelItem();
    if (!solidTop || checkedTops.contains(solidTop))
      continue;
    checkedTops.insert(solidTop);

    QRectF compRect = getComponentBodyRect(solidTop);
    if (compRect.isEmpty())
      continue;

    for (int i = 0; i + 1 < segments.size(); ++i) {
      QPointF p1 = wire->mapToScene(segments[i]);
      QPointF p2 = wire->mapToScene(segments[i + 1]);
      QLineF seg(p1, p2);
      QPointF ip;
      QLineF top(compRect.topLeft(), compRect.topRight());
      QLineF bottom(compRect.bottomLeft(), compRect.bottomRight());
      QLineF left(compRect.topLeft(), compRect.bottomLeft());
      QLineF right(compRect.topRight(), compRect.bottomRight());

      if (compRect.contains(p1) || compRect.contains(p2) ||
          seg.intersects(top, &ip) == QLineF::BoundedIntersection ||
          seg.intersects(bottom, &ip) == QLineF::BoundedIntersection ||
          seg.intersects(left, &ip) == QLineF::BoundedIntersection ||
          seg.intersects(right, &ip) == QLineF::BoundedIntersection) {
        return false; // Wire intersects a solid component body!
      }
    }
  }
  return true;
}
