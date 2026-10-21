#ifndef GROUPSELECTIONBOX_H
#define GROUPSELECTIONBOX_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QList>
#include <QRectF>

class CircuitScene;

/**
 * GroupSelectionBox
 *
 * A dashed rectangle drawn around a group of selected circuit items.
 * - Rendered as a dashed blue border with a soft translucent blue fill.
 * - Allows dragging either the components directly or dragging the box
 *   to move the entire selected circuit across the canvas.
 * - Deleting with Backspace or Delete removes all selected components and wires.
 * - No extra buttons required on screen.
 */
class GroupSelectionBox : public QGraphicsItem {
public:
    explicit GroupSelectionBox(QGraphicsScene *scene = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    /// Recalculates the bounding rectangle enclosing all selected items.
    /// Returns false if fewer than 2 items are selected.
    bool recalculate();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QRectF m_rect;
    bool m_dragging = false;
    QPointF m_dragStartScene;
    QList<QGraphicsItem *> m_dragItems;
    QList<QPointF> m_dragItemStartPositions;
};

#endif // GROUPSELECTIONBOX_H

