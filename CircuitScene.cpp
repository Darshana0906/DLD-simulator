#include "CircuitScene.h"
#include "SelectionBox.h"
#include "elements/Point.h"
#include "elements/Wire.h"
#include <QPainter>
#include <QLineF>

CircuitScene::CircuitScene(QObject *parent)
    : QGraphicsScene(parent), m_placementMode(false), m_activeSelectionBox(nullptr) {
    setBackgroundBrush(QColor(248, 249, 250));

    connect(this, &QGraphicsScene::selectionChanged, this, &CircuitScene::onSelectionChanged);
}

CircuitScene::~CircuitScene() {
    clearWirePlacement();
    clearSelectionBox();
}

void CircuitScene::setPlacementMode(bool active) {
    m_placementMode = active;
    if (active) {
        clearSelection();
    } else {
        clearWirePlacement();
    }
}

bool CircuitScene::isPlacementMode() const {
    return m_placementMode || m_wireToolActive;
}

void CircuitScene::setWireToolActive(bool active) {
    m_wireToolActive = active;
    if (active) {
        clearSelection();
    } else {
        clearWirePlacement();
    }
}

Point* CircuitScene::findPinNear(const QPointF &scenePos, qreal maxDistance) {
    QRectF searchRect(scenePos.x() - maxDistance, scenePos.y() - maxDistance, maxDistance * 2, maxDistance * 2);
    QList<QGraphicsItem*> itemsInRect = items(searchRect);

    Point *closestPin = nullptr;
    qreal minDistance = maxDistance;

    for (QGraphicsItem *item : itemsInRect) {
        if (Point *pin = dynamic_cast<Point*>(item)) {
            qreal dist = QLineF(scenePos, pin->getCenterScenePos()).length();
            if (dist < minDistance) {
                minDistance = dist;
                closestPin = pin;
            }
        }
    }
    return closestPin;
}

void CircuitScene::clearSelectionBox() {
    if (m_activeSelectionBox) {
        delete m_activeSelectionBox;
        m_activeSelectionBox = nullptr;
    }
}

void CircuitScene::clearWirePlacement() {
    if (m_hoveredPin) {
        m_hoveredPin->setHighlighted(false);
        m_hoveredPin = nullptr;
    }
    if (m_wireStartPoint) {
        m_wireStartPoint->setSelectedForWire(false);
        m_wireStartPoint = nullptr;
    }
    if (m_draftWire) {
        removeItem(m_draftWire);
        delete m_draftWire;
        m_draftWire = nullptr;
    }
}

void CircuitScene::onPointClicked(Point *point) {
    if (!point) return;

    if (m_wireStartPoint == nullptr) {
        // Step 1: Select start pin for wire connection
        m_wireStartPoint = point;
        m_wireStartPoint->setSelectedForWire(true);

        m_draftWire = new Wire(m_wireStartPoint);
        addItem(m_draftWire);
    } else if (m_wireStartPoint != point) {
        // Step 2: Connect to target pin
        Point *start = m_wireStartPoint;

        clearWirePlacement(); // Clears draft wire and resets start pin state

        // Create permanent Manhattan Wire between start pin and target pin
        Wire *wire = new Wire(start, point);
        addItem(wire);

        // Automatically exit Wire tool mode after completing 2-pin connection
        emit placementCancelled();
    }
}

void CircuitScene::onSelectionChanged() {
    clearSelectionBox();

    QList<QGraphicsItem *> selected = selectedItems();
    if (selected.size() == 1) {
        QGraphicsItem *target = selected.first();
        if (!dynamic_cast<Point *>(target) && !dynamic_cast<Wire *>(target) && target != m_activeSelectionBox) {
            m_activeSelectionBox = new SelectionBox(target);
        }
    }
}

void CircuitScene::deleteItem(QGraphicsItem *item) {
    if (!item)
        return;

    if (m_activeSelectionBox && (m_activeSelectionBox == item || m_activeSelectionBox->parentItem() == item)) {
        clearSelectionBox();
    }

    removeItem(item);
    delete item;
}

void CircuitScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        if (m_wireStartPoint) {
            clearWirePlacement();
            event->accept();
            return;
        }

        if (m_wireToolActive || m_placementMode) {
            emit placementCancelled();
            event->accept();
            return;
        }

        QGraphicsItem *itemAtPos = itemAt(event->scenePos(), QTransform());
        if (itemAtPos) {
            if (m_activeSelectionBox && itemAtPos == m_activeSelectionBox) {
                itemAtPos = m_activeSelectionBox->parentItem();
            }

            if (itemAtPos && itemAtPos->isSelected()) {
                deleteItem(itemAtPos);
                event->accept();
                return;
            }
        }
    }

    if (m_wireToolActive) {
        if (event->button() == Qt::LeftButton) {
            Point *pin = findPinNear(event->scenePos(), 25.0);
            if (pin) {
                onPointClicked(pin);
            }
            event->accept();
            return;
        }
    }

    if (m_placementMode) {
        if (event->button() == Qt::LeftButton) {
            emit sceneClicked(event->scenePos());
            event->accept();
            return;
        }
    }

    QGraphicsScene::mousePressEvent(event);
}

void CircuitScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    Point *nearbyPin = findPinNear(event->scenePos(), 25.0);

    // Highlight hovered pin
    if (nearbyPin != m_hoveredPin) {
        if (m_hoveredPin) {
            m_hoveredPin->setHighlighted(false);
        }
        m_hoveredPin = nearbyPin;
        if (m_hoveredPin) {
            m_hoveredPin->setHighlighted(true);
        }
    }

    // Update draft wire rubberband preview position (snap to nearby pin if present)
    if (m_draftWire) {
        if (nearbyPin && nearbyPin != m_wireStartPoint) {
            m_draftWire->setPreviewPos(nearbyPin->getCenterScenePos());
        } else {
            m_draftWire->setPreviewPos(event->scenePos());
        }
    }

    QGraphicsScene::mouseMoveEvent(event);
}

void CircuitScene::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        if (m_wireStartPoint) {
            clearWirePlacement();
            event->accept();
            return;
        }
        if (m_wireToolActive || m_placementMode) {
            emit placementCancelled();
            event->accept();
            return;
        }
    }

    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        QList<QGraphicsItem *> selected = selectedItems();
        for (QGraphicsItem *item : selected) {
            deleteItem(item);
        }
        event->accept();
        return;
    }

    QGraphicsScene::keyPressEvent(event);
}

void CircuitScene::drawBackground(QPainter *painter, const QRectF &rect) {
    QGraphicsScene::drawBackground(painter, rect);

    const int gridSize = 20;
    qreal left = int(rect.left()) - (int(rect.left()) % gridSize);
    qreal top = int(rect.top()) - (int(rect.top()) % gridSize);

    QVector<QPointF> points;
    int cols = int((rect.right() - left) / gridSize) + 1;
    int rows = int((rect.bottom() - top) / gridSize) + 1;
    points.reserve(cols * rows);

    for (qreal x = left; x < rect.right(); x += gridSize) {
        for (qreal y = top; y < rect.bottom(); y += gridSize) {
            points.append(QPointF(x, y));
        }
    }

    painter->setPen(QPen(QColor(210, 215, 222), 1));
    painter->drawPoints(points.constData(), points.size());
}
