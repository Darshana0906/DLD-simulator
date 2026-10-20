#include "GroupSelectionBox.h"
#include "elements/Point.h"
#include "elements/Wire.h"
#include "SelectionBox.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QCursor>

// ─── helpers ────────────────────────────────────────────────────────────────────

/// Return true if this is a "top-level selectable component" (not a Wire,
/// Point, or decoration).
static bool isSelectableComponent(QGraphicsItem *item) {
    if (!item) return false;
    if (dynamic_cast<Wire *>(item))         return false;
    if (dynamic_cast<Point *>(item))        return false;
    if (dynamic_cast<SelectionBox *>(item)) return false;
    if (dynamic_cast<GroupSelectionBox *>(item)) return false;
    return (item->flags() & QGraphicsItem::ItemIsSelectable);
}

// ─── construction ───────────────────────────────────────────────────────────────

GroupSelectionBox::GroupSelectionBox(QGraphicsScene *scene)
    : QGraphicsItem(nullptr) {
    Q_UNUSED(scene);
    setZValue(999);                            // Render above most items
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setAcceptHoverEvents(false);
    setCursor(Qt::SizeAllCursor);
}

// ─── geometry ───────────────────────────────────────────────────────────────────

QRectF GroupSelectionBox::boundingRect() const {
    // Pad a little so the border is not clipped
    return m_rect.adjusted(-4, -4, 4, 4);
}

bool GroupSelectionBox::recalculate() {
    if (!scene()) return false;

    QList<QGraphicsItem *> sel = scene()->selectedItems();

    // Collect top-level selectable components
    QList<QGraphicsItem *> components;
    for (QGraphicsItem *item : sel) {
        QGraphicsItem *top = item->topLevelItem() ? item->topLevelItem() : item;
        if (isSelectableComponent(top) && !components.contains(top)) {
            components.append(top);
        }
    }

    // Also collect any selected standalone wires
    for (QGraphicsItem *item : sel) {
        if (Wire *w = dynamic_cast<Wire *>(item)) {
            if (!components.contains(w))
                components.append(w);
        }
    }

    if (components.size() < 2) return false;

    // Compute union of all scene bounding rects
    QRectF united;
    for (QGraphicsItem *item : components) {
        if (united.isNull())
            united = item->sceneBoundingRect();
        else
            united = united.united(item->sceneBoundingRect());
    }

    prepareGeometryChange();
    const qreal padding = 12.0;
    m_rect = united.adjusted(-padding, -padding, padding, padding);
    update();
    return true;
}

// ─── painting ───────────────────────────────────────────────────────────────────

void GroupSelectionBox::paint(QPainter *painter,
                              const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setRenderHint(QPainter::Antialiasing);

    // Semi-transparent blue fill
    painter->setBrush(QBrush(QColor(33, 150, 243, 25)));

    // Dashed blue border
    QPen pen(QColor(33, 150, 243, 180), 2.0, Qt::DashLine);
    pen.setDashPattern({6, 4});
    painter->setPen(pen);
    painter->drawRoundedRect(m_rect, 6, 6);
}

// ─── drag to move the group ─────────────────────────────────────────────────────

void GroupSelectionBox::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_rect.contains(event->scenePos())) {
        m_dragging = true;
        m_dragStartScene = event->scenePos();

        // Snapshot the selected top-level items and their positions
        m_dragItems.clear();
        m_dragItemStartPositions.clear();

        if (scene()) {
            QList<QGraphicsItem *> sel = scene()->selectedItems();
            QSet<QGraphicsItem *> seen;
            for (QGraphicsItem *item : sel) {
                QGraphicsItem *top = item->topLevelItem() ? item->topLevelItem() : item;
                if (seen.contains(top)) continue;
                // Skip wires (they follow their endpoints automatically)
                if (dynamic_cast<Wire *>(top)) continue;
                if (dynamic_cast<GroupSelectionBox *>(top)) continue;
                if (dynamic_cast<SelectionBox *>(top)) continue;
                if (dynamic_cast<Point *>(top)) continue;
                seen.insert(top);
                m_dragItems.append(top);
                m_dragItemStartPositions.append(top->pos());
            }
        }

        event->accept();
        return;
    }
    event->ignore();
}

void GroupSelectionBox::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (m_dragging) {
        QPointF delta = event->scenePos() - m_dragStartScene;

        // Move all items
        for (int i = 0; i < m_dragItems.size(); ++i) {
            QGraphicsItem *item = m_dragItems[i];
            if (item && scene() && scene()->items().contains(item)) {
                item->setPos(m_dragItemStartPositions[i] + delta);
            }
        }

        // Update the box position
        recalculate();
        event->accept();
        return;
    }
    event->ignore();
}

void GroupSelectionBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (m_dragging) {
        m_dragging = false;
        m_dragItems.clear();
        m_dragItemStartPositions.clear();
        recalculate();
        event->accept();
        return;
    }
    event->ignore();
}

