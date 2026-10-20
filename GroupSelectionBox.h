#ifndef GROUPSELECTIONBOX_H
#define GROUPSELECTIONBOX_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QList>

class CircuitScene;

/**
 * GroupSelectionBox
 *
 * A translucent rectangle drawn around a group of selected items.
 * - Painted as a dashed blue border with a light-blue semi-transparent fill.
 * - Allows the user to drag the rectangle to move the entire group.
 * - No buttons on screen — pressing Backspace/Delete deletes the group
 *   (handled by CircuitScene::keyPressEvent which already calls deleteSelectedItems).
 */
class GroupSelectionBox : public QGraphicsItem {
public:
    explicit GroupSelectionBox(QGraphicsScene *scene);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    /// Recalculate the bounding rectangle from the current scene selection.
    /// Returns false if there are fewer than 2 items selected (box should be removed).
    bool recalculate();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QRectF m_rect;            // Scene-coordinate bounding rect of the group
    bool m_dragging = false;
    QPointF m_dragStartScene;
    QList<QGraphicsItem *> m_dragItems;       // Snapshot of items being dragged
    QList<QPointF> m_dragItemStartPositions;  // Their original positions
};

#endif // GROUPSELECTIONBOX_H

