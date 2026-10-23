#include "Gate.h"
#include "../CircuitScene.h"
#include "../overlapping.h"
#include "../elements/Wire.h"
#include "../logic/CircuitAdapter.h"
#include "../logic/Node.h"
#include <QPainter>
#include <QInputDialog>

Gate::Gate(QGraphicsItem *parent) : QGraphicsItem(parent) {
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

void Gate::addInputPin(qreal x, qreal y) {
  Point *pin = new Point(this, false, PinType::Input);
  pin->setPos(x, y);
  m_inputPins.append(pin);
}

void Gate::setOutputPin(qreal x, qreal y) {
  if (m_outputPin) {
    delete m_outputPin;
  }
  m_outputPin = new Point(this, false, PinType::Output);
  m_outputPin->setPos(x, y);
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


QVariant Gate::itemChange(GraphicsItemChange change, const QVariant &value) {
  if (change == ItemPositionChange && scene()) {
    QPointF newPos = value.toPointF();
    QRectF newSceneRect =
        mapToScene(boundingRect()).boundingRect().translated(newPos - pos());
    if (!canMoveItem(this, newSceneRect)) {
      return pos(); // Reject movement!
    }
  }

  if (change == ItemPositionHasChanged || change == ItemTransformHasChanged || change == ItemScaleHasChanged) {
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

void Gate::setInputCount(int count) {
    if (count < 1) return;
    int oldCount = m_inputPins.size();
    if (oldCount == count) return;

    prepareGeometryChange();

    // Evenly space pins between y = 10 and y = 50
    qreal startY = 10.0;
    qreal endY = 50.0;
    qreal step = (count > 1) ? (endY - startY) / (count - 1) : 0;

    // 1. If increasing: keep existing pins, add new pins
    while (m_inputPins.size() < count) {
        addInputPin(-29, 0);
    }

    // 2. If decreasing: remove excess pins from the end
    while (m_inputPins.size() > count) {
        Point *excessPin = m_inputPins.takeLast();
        if (CircuitScene *cs = dynamic_cast<CircuitScene*>(scene())) {
            QList<Wire*> wires = excessPin->getWires();
            for (Wire *w : wires) {
                cs->deleteItem(w);
            }
        }
        delete excessPin;
    }

    // 3. Reposition all pins to their new locations
    for (int i = 0; i < count; ++i) {
        qreal centerY = startY + i * step;
        qreal pinY = centerY - 5.0;
        m_inputPins[i]->setPos(-29, pinY);
    }

    // 4. Update all connected wires on input and output pins so paths adapt immediately
    for (Point *pin : m_inputPins) {
        for (Wire *wire : pin->getWires()) {
            wire->updatePath();
        }
    }
    if (m_outputPin) {
        for (Wire *wire : m_outputPin->getWires()) {
            wire->updatePath();
        }
    }

    // 5. Update logic node in CircuitAdapter and re-extract edges
    if (CircuitScene *cs = dynamic_cast<CircuitScene *>(scene())) {
        if (CircuitAdapter *adapter = cs->getAdapter()) {
            if (GateNode *gn = adapter->getGateNode(this)) {
                gn->setInputCount(static_cast<size_t>(count));
            }
            adapter->extractWires();
            emit adapter->simulationInvalidated();
        }
    }

    update();
}

void Gate::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    QInputDialog dialog;

    dialog.setWindowTitle("Gate Inputs");
    dialog.setLabelText("Number of inputs (2 to 8):");
    dialog.setInputMode(QInputDialog::IntInput);
    dialog.setIntValue(m_inputPins.size());
    dialog.setIntMinimum(2);
    dialog.setIntMaximum(8);
    dialog.setIntStep(1);

    dialog.resize(400, 120);
  

    if (dialog.exec() == QDialog::Accepted) {
        int oldCount = m_inputPins.size();
        int newCount = dialog.intValue();
        if (oldCount != newCount) {
            setInputCount(newCount);
            if (CircuitScene *cs = dynamic_cast<CircuitScene *>(scene())) {
                emit cs->circuitModified();
            }
        }
    }
}
