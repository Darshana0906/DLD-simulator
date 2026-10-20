#include "CircuitScene.h"
#include "SelectionBox.h"
#include "GroupSelectionBox.h"
#include "overlapping.h"
#include "elements/Point.h"
#include "elements/Wire.h"
#include "elements/Input.h"
#include "elements/LED.h"
#include "gates/Gate.h"
#include "logic/CircuitAdapter.h"
#include <QPainter>
#include <QLineF>

CircuitScene::CircuitScene(QObject *parent)
    : QGraphicsScene(parent), m_placementMode(false), m_activeSelectionBox(nullptr) {
    setBackgroundBrush(QColor(248, 249, 250));

    m_adapter = new CircuitAdapter(this, this);

    connect(this, &QGraphicsScene::selectionChanged, this, &CircuitScene::onSelectionChanged);
}

CircuitScene::~CircuitScene() {
    clearWirePlacement();
    clearGroupSelectionBox();
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

void CircuitScene::clearGroupSelectionBox() {
    if (m_groupSelectionBox) {
        if (items().contains(m_groupSelectionBox))
            removeItem(m_groupSelectionBox);
        delete m_groupSelectionBox;
        m_groupSelectionBox = nullptr;
    }
}

void CircuitScene::updateGroupSelectionBox() {
    // Count selected top-level components (excluding wires/points/decorations)
    QList<QGraphicsItem *> sel = selectedItems();
    int componentCount = 0;
    QSet<QGraphicsItem *> seen;
    for (QGraphicsItem *item : sel) {
        QGraphicsItem *top = item->topLevelItem() ? item->topLevelItem() : item;
        if (seen.contains(top)) continue;
        if (dynamic_cast<Wire *>(top)) continue;
        if (dynamic_cast<Point *>(top)) continue;
        if (dynamic_cast<SelectionBox *>(top)) continue;
        if (dynamic_cast<GroupSelectionBox *>(top)) continue;
        seen.insert(top);
        componentCount++;
    }

    if (componentCount >= 2) {
        // Create or update the group selection box
        if (!m_groupSelectionBox) {
            m_groupSelectionBox = new GroupSelectionBox(this);
            addItem(m_groupSelectionBox);
        }
        if (!m_groupSelectionBox->recalculate()) {
            clearGroupSelectionBox();
        }
    } else {
        clearGroupSelectionBox();
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

        QGraphicsItem *ownerStart = start->parentItem() ? start->parentItem() : start->topLevelItem();
        QGraphicsItem *ownerEnd   = point->parentItem() ? point->parentItem() : point->topLevelItem();

        bool startIsOutput = false;
        bool endIsOutput = false;

        if (m_adapter) {
            int idStart = -1, idEnd = -1;
            int pinIdxStart = 0, pinIdxEnd = 0;
            bool okStart = m_adapter->getPinInfo(start, idStart, startIsOutput, pinIdxStart);
            bool okEnd   = m_adapter->getPinInfo(point, idEnd, endIsOutput, pinIdxEnd);
            if (!okStart) startIsOutput = (start->getPinType() == PinType::Output);
            if (!okEnd)   endIsOutput   = (point->getPinType() == PinType::Output);
        } else {
            startIsOutput = (start->getPinType() == PinType::Output);
            endIsOutput   = (point->getPinType() == PinType::Output);
        }

        // Rule 1: Output-to-Output is forbidden
        if (startIsOutput && endIsOutput) {
            clearWirePlacement();
            bool hasGate = (dynamic_cast<Gate*>(ownerStart) || dynamic_cast<Gate*>(ownerEnd));
            bool hasInput = (dynamic_cast<Input*>(ownerStart) || dynamic_cast<Input*>(ownerEnd));
            if (hasGate && hasInput) {
                emit errorMessage("Cannot connect Switch / GND / Constant to the output of a gate! Both are outputs.");
            } else if (hasGate) {
                emit errorMessage("Cannot connect two gate outputs together!");
            } else {
                emit errorMessage("Cannot connect two output pins together!");
            }
            emit placementCancelled();
            return;
        }

        // Rule 2: Input-to-Input is forbidden
        if (!startIsOutput && !endIsOutput) {
            clearWirePlacement();
            bool hasGate = (dynamic_cast<Gate*>(ownerStart) || dynamic_cast<Gate*>(ownerEnd));
            bool hasBulb = (dynamic_cast<LED*>(ownerStart) || dynamic_cast<LED*>(ownerEnd));
            if (hasGate && hasBulb) {
                emit errorMessage("Cannot connect Bulb (LED) to the input of a gate! Both are inputs.");
            } else if (hasGate) {
                emit errorMessage("Cannot connect two gate inputs directly together!");
            } else {
                emit errorMessage("Cannot connect two input pins together!");
            }
            emit placementCancelled();
            return;
        }

        // Rule 3: An input pin can only receive one driving wire
        Point *inputPin = startIsOutput ? point : start;
        if (!inputPin->getWires().isEmpty()) {
            clearWirePlacement();
            emit errorMessage("This input pin already has a wire connected to it!");
            emit placementCancelled();
            return;
        }

        clearWirePlacement(); // Clears draft wire and resets start pin state

        // Create permanent Manhattan Wire between start pin and target pin
        Wire *wire = new Wire(start, point);
        addItem(wire);

        if (!canPlaceWire(wire)) {
            removeItem(wire);
            delete wire;
            emit errorMessage("Wire cannot cross through a gate or component!");
            return;
        }

        // Notify the adapter so it registers the edge in the logic graph
        if (m_adapter) {
            m_adapter->onWireConnected(wire);
        }
    }
}

void CircuitScene::onSelectionChanged() {
    clearSelectionBox();

    QList<QGraphicsItem *> selected = selectedItems();
    if (selected.size() == 1) {
        QGraphicsItem *target = selected.first();
        if (!dynamic_cast<Point *>(target) && !dynamic_cast<Wire *>(target) && target != m_activeSelectionBox
            && !dynamic_cast<GroupSelectionBox *>(target)) {
            m_activeSelectionBox = new SelectionBox(target);
        }
    }

    // Manage group selection box for multi-selection
    updateGroupSelectionBox();
}

void CircuitScene::deleteItem(QGraphicsItem *item) {
    if (!item || !items().contains(item))
        return;

    if (m_activeSelectionBox && (m_activeSelectionBox == item || m_activeSelectionBox->parentItem() == item)) {
        clearSelectionBox();
    }

    if (m_hoveredPin && (m_hoveredPin == item || m_hoveredPin->topLevelItem() == item->topLevelItem())) {
        m_hoveredPin = nullptr;
    }
    if (m_wireStartPoint && (m_wireStartPoint == item || m_wireStartPoint->topLevelItem() == item->topLevelItem())) {
        clearWirePlacement();
    }

    // Collect all wires attached to this item's pins, before removing
    QList<Wire*> attachedWires;
    for (QGraphicsItem *child : item->childItems()) {
        if (Point *pin = dynamic_cast<Point*>(child)) {
            for (Wire *w : pin->getWires()) {
                if (w && !attachedWires.contains(w)) {
                    attachedWires.append(w);
                }
            }
        }
    }
    // Delete attached wires first
    for (Wire *w : attachedWires) {
        if (w && items().contains(w)) {
            if (m_adapter) m_adapter->onWireRemoved(w);
            removeItem(w);
            delete w;
        }
    }

    if (m_adapter) {
        if (Wire *wire = dynamic_cast<Wire*>(item)) {
            m_adapter->onWireRemoved(wire);
        } else {
            m_adapter->unregisterItem(item);
        }
    }

    removeItem(item);
    delete item;
}

void CircuitScene::clearCircuit() {
    clearWirePlacement();
    clearSelectionBox();
    clear();
    if (m_adapter) {
        m_adapter->clear();
    }
}

void CircuitScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // ── Right-click: cancel wire / placement, or delete hovered item ────────────
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
        // Right-click on a selected item → delete the whole selection
        QGraphicsItem *hitItem = itemAt(event->scenePos(), QTransform());
        if (hitItem) {
            if (m_activeSelectionBox && hitItem == m_activeSelectionBox)
                hitItem = m_activeSelectionBox->parentItem();
            else
                hitItem = hitItem->topLevelItem() ? hitItem->topLevelItem() : hitItem;

            if (hitItem && hitItem->isSelected()) {
                deleteSelectedItems();
                event->accept();
                return;
            }
        }
    }

    // ── Wire tool: click pins ────────────────────────────────────────────────────
    if (m_wireToolActive && event->button() == Qt::LeftButton) {
        Point *pin = findPinNear(event->scenePos(), 25.0);
        if (pin) {
            onPointClicked(pin);
        }
        event->accept();
        return;
    }

    // ── Placement mode: emit position for item placement ────────────────────────
    if (m_placementMode && event->button() == Qt::LeftButton) {
        emit sceneClicked(event->scenePos());
        event->accept();
        return;
    }

    // ── Normal (select / move) mode ──────────────────────────────────────────────
    if (event->button() == Qt::LeftButton) {
        QGraphicsItem *hit = itemAt(event->scenePos(), QTransform());

        // If the user clicked on the group selection box, let it handle dragging
        if (m_groupSelectionBox && hit == m_groupSelectionBox) {
            QGraphicsScene::mousePressEvent(event);
            return;
        }

        if (!hit) {
            // Clicked on empty canvas → clear selection and let rubberband start
            clearSelection();
            QGraphicsScene::mousePressEvent(event);
            return;
        }

        // Resolve to top-level item
        QGraphicsItem *target = hit;
        if (m_activeSelectionBox && hit == m_activeSelectionBox)
            target = m_activeSelectionBox->parentItem();
        else if (hit->topLevelItem())
            target = hit->topLevelItem();

        if (!target || !(target->flags() & QGraphicsItem::ItemIsSelectable)) {
            QGraphicsScene::mousePressEvent(event);
            return;
        }

        bool shift = event->modifiers() & Qt::ShiftModifier;

        if (target->isSelected()) {
            // Clicking an already-selected item: KEEP the whole selection so
            // the user can drag the entire group. Just forward to base class
            // for drag initiation.
            QGraphicsScene::mousePressEvent(event);
            return;
        }

        if (shift) {
            // Shift-click: add/remove this item from the selection
            target->setSelected(!target->isSelected());
            event->accept();
            return;
        }

        // Normal click on unselected item: select only this item
        clearSelection();
        target->setSelected(true);
        QGraphicsScene::mousePressEvent(event);
        return;
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
        if (!selectedItems().isEmpty()) {
            clearSelection();
            event->accept();
            return;
        }
    }

    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        deleteSelectedItems();
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
