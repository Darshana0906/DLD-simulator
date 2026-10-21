#include "GroupSelectionBox.h"
#include "elements/Point.h"
#include "elements/Wire.h"
#include "SelectionBox.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QCursor>

static bool isSelectableItem(QGraphicsItem *item) {
    if (!item) return false;
    if (dynamic_cast<Wire *>(item))         return false;
    if (dynamic_cast<Point *>(item))        return false;
    if (dynamic_cast<SelectionBox *>(item)) return false;
    if (dynamic_cast<GroupSelectionBox *>(item)) return false;
    return (item->flags() & QGraphicsItem::ItemIsSelectable);
}

GroupSelectionBox::GroupSelectionBox(QGraphicsScene *scene)
    : QGraphicsItem(nullptr) {
    Q_UNUSED(scene);
    setZValue(-0.5); // Place just behind components so components remain interactive
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setAcceptHoverEvents(false);
    setCursor(Qt::SizeAllCursor);
}

QRectF GroupSelectionBox::boundingRect() const {
    if (m_rect.isNull() || !m_rect.isValid())
        return QRectF();
    return m_rect.adjusted(-6, -6, 6, 6);
}

QPainterPath GroupSelectionBox::shape() const {
    QPainterPath path;
    if (m_rect.isValid() && !m_rect.isEmpty()) {
        path.addRoundedRect(m_rect, 6, 6);
    }
    return path;
}

bool GroupSelectionBox::recalculate() {
    if (!scene()) return false;

    QList<QGraphicsItem *> sel = scene()->selectedItems();

    // Collect top-level selectable components and standalone wires
    QList<QGraphicsItem *> components;
    for (QGraphicsItem *item : sel) {
        if (!item) continue;
        QGraphicsItem *top = item->topLevelItem() ? item->topLevelItem() : item;
        if (top == this) continue;
        if (dynamic_cast<SelectionBox *>(top)) continue;
        if (dynamic_cast<Point *>(top)) continue;

        if (dynamic_cast<Wire *>(top)) {
            if (!components.contains(top)) components.append(top);
        } else if (isSelectableItem(top)) {
            if (!components.contains(top)) components.append(top);
        }
    }

    if (components.size() < 2) {
        prepareGeometryChange();
        m_rect = QRectF();
        update();
        return false;
    }

    // Compute union of scene bounding rects
    QRectF united;
    for (QGraphicsItem *item : components) {
        if (united.isNull())
            united = item->sceneBoundingRect();
        else
            united = united.united(item->sceneBoundingRect());
    }

    if (united.isNull() || !united.isValid()) {
        prepareGeometryChange();
        m_rect = QRectF();
        update();
        return false;
    }

    prepareGeometryChange();
    const qreal padding = 14.0;
    m_rect = united.adjusted(-padding, -padding, padding, padding);
    update();
    return true;
}

void GroupSelectionBox::paint(QPainter *painter,
                              const QStyleOptionGraphicsItem *, QWidget *) {
    if (m_rect.isNull() || !m_rect.isValid())
        return;

    painter->setRenderHint(QPainter::Antialiasing);

    // Subtle translucent fill
    painter->setBrush(QBrush(QColor(33, 150, 243, 20)));

    // Dashed blue border
    QPen pen(QColor(25, 118, 210, 200), 2.0, Qt::DashLine);
    pen.setDashPattern({6, 4});
    painter->setPen(pen);
    painter->drawRoundedRect(m_rect, 8, 8);
}

void GroupSelectionBox::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_rect.contains(event->scenePos())) {
        m_dragging = true;
        m_dragStartScene = event->scenePos();

        m_dragItems.clear();
        m_dragItemStartPositions.clear();

        if (scene()) {
            QList<QGraphicsItem *> sel = scene()->selectedItems();
            QSet<QGraphicsItem *> seen;
            for (QGraphicsItem *item : sel) {
                if (!item) continue;
                QGraphicsItem *top = item->topLevelItem() ? item->topLevelItem() : item;
                if (!top || seen.contains(top)) continue;
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

        for (int i = 0; i < m_dragItems.size(); ++i) {
            QGraphicsItem *item = m_dragItems[i];
            if (item && scene() && scene()->items().contains(item)) {
                item->setPos(m_dragItemStartPositions[i] + delta);
            }
        }

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

