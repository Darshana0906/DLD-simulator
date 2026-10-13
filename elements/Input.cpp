#include "Input.h"
#include "../CircuitScene.h"
#include "../overlapping.h"
#include "Point.h"
#include "Wire.h"

#include <QPainter>

Input::Input(bool value, QGraphicsItem *parent)
    : QGraphicsItem(parent), value(value) {
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  outputPoint = new Point(this, false);
  outputPoint->setPos(55, 20);
}

bool Input::getValue() const { return value; }

void Input::setValue(bool value) {
  this->value = value;
  update();
}

LogicState Input::getLogicState() const {
  return value ? LogicState::HIGH : LogicState::LOW;
}

Point *Input::getOutputPoint() const { return outputPoint; }

QRectF Input::boundingRect() const { return QRectF(0, 0, 65, 50); }

void Input::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
                  QWidget *) {
  painter->setPen(Qt::black);
  if (value)
    painter->setBrush(Qt::green);
  else
    painter->setBrush(Qt::darkGray);

  painter->drawRect(10, 10, 40, 30);
  painter->drawLine(50, 25, 60, 25);
}

void Input::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  CircuitScene *cs = dynamic_cast<CircuitScene *>(scene());
  if (cs && cs->isPlacementMode()) {
    event->ignore();
    return;
  }
  QGraphicsItem::mousePressEvent(event);
}

#include "../overlapping.h"

QVariant Input::itemChange(GraphicsItemChange change, const QVariant &value) {
  if (change == ItemPositionChange && scene()) {
    QPointF newPos = value.toPointF();
    QRectF newSceneRect =
        mapToScene(boundingRect()).boundingRect().translated(newPos - pos());
    if (!canMoveItem(this, newSceneRect)) {
      return pos(); // Reject movement!
    }
  }

  if ((change == ItemPositionHasChanged || change == ItemTransformHasChanged) &&
      outputPoint) {
    for (Wire *wire : outputPoint->getWires()) {
      wire->updatePath();
    }
  }
  return QGraphicsItem::itemChange(change, value);
}
