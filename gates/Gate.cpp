#include "Gate.h"
#include "../CircuitScene.h"
#include "../overlapping.h"
#include <QPainter>

Gate::Gate(QGraphicsItem *parent) : QGraphicsItem(parent) {
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

QVariant Gate::itemChange(GraphicsItemChange change, const QVariant &value) {
  if (change == QGraphicsItem::ItemPositionChange && scene()) {
    const QPointF delta = parentItem()
                              ? parentItem()->mapToScene(value.toPointF()) -
                                    parentItem()->mapToScene(pos())
                              : value.toPointF() - pos();
    if (!canMoveItem(this, sceneBoundingRect().translated(delta)))
      return pos();
  }
  return QGraphicsItem::itemChange(change, value);
}

QRectF Gate::boundingRect() const { return QRectF(-35, -5, 170, 70); }

void Gate::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
                 QWidget *) {
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

#include "../overlapping.h"

QVariant Gate::itemChange(GraphicsItemChange change, const QVariant &value) {
  if (change == ItemPositionChange && scene()) {
    QPointF newPos = value.toPointF();
    QRectF newSceneRect =
        mapToScene(boundingRect()).boundingRect().translated(newPos - pos());
    if (!canMoveItem(this, newSceneRect)) {
      return pos(); // Reject movement!
    }
  }

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
