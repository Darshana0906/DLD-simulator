#include "CircuitScene.h"
#include "SelectionBox.h"
#include "elements/Point.h"
#include "elements/Wire.h"
#include <QPainter>

CircuitScene::CircuitScene(QObject *parent)
    : QGraphicsScene(parent), m_placementMode(false), m_activeSelectionBox(nullptr) {
    setBackgroundBrush(QColor(248, 249, 250));

    connect(this, &QGraphicsScene::selectionChanged, this, &CircuitScene::onSelectionChanged);
}

CircuitScene::~CircuitScene() {
    clearSelectionBox();
}

void CircuitScene::setPlacementMode(bool active) {
    m_placementMode = active;
    if (active) {
        clearSelection();
    }
}

bool CircuitScene::isPlacementMode() const {
    return m_placementMode;
}

void CircuitScene::clearSelectionBox() {
    if (m_activeSelectionBox) {
        delete m_activeSelectionBox;
        m_activeSelectionBox = nullptr;
    }
}

void CircuitScene::onSelectionChanged() {
    clearSelectionBox();

    QList<QGraphicsItem *> selected = selectedItems();
    if (selected.size() == 1) {
        QGraphicsItem *target = selected.first();
        // Attach selection box and corner resize handles to components (excluding individual Points and Wires)
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
        if (m_placementMode) {
            emit placementCancelled();
            event->accept();
            return;
        }

        // Check if right-clicking on an already selected item
        QGraphicsItem *itemAtPos = itemAt(event->scenePos(), QTransform());
        if (itemAtPos) {
            if (m_activeSelectionBox && itemAtPos == m_activeSelectionBox) {
                itemAtPos = m_activeSelectionBox->parentItem();
            }

            // User must first select the element, then right-click to delete it!
            if (itemAtPos && itemAtPos->isSelected()) {
                deleteItem(itemAtPos);
                event->accept();
                return;
            }
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

void CircuitScene::keyPressEvent(QKeyEvent *event) {
    if (m_placementMode && event->key() == Qt::Key_Escape) {
        emit placementCancelled();
        event->accept();
        return;
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

    // Draw subtle grid dots every 20 pixels — batched for performance
    const int gridSize = 20;
    qreal left = int(rect.left()) - (int(rect.left()) % gridSize);
    qreal top = int(rect.top()) - (int(rect.top()) % gridSize);

    QVector<QPointF> points;
    // Pre-allocate to avoid repeated heap allocations inside the loop
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
