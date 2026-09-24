#ifndef SELECTIONBOX_H
#define SELECTIONBOX_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>

class SelectionBox : public QGraphicsItem {
public:
    enum Handle {
        HandleNone = 0,
        HandleTopLeft,
        HandleTopRight,
        HandleBottomLeft,
        HandleBottomRight
    };

    explicit SelectionBox(QGraphicsItem *targetItem);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    Handle hitTestHandle(const QPointF &pos) const;
    QRectF getHandleRect(const QPointF &center, qreal size) const;

    QGraphicsItem *m_targetItem;
    Handle m_activeHandle = HandleNone;
    bool m_isResizing = false;
    qreal m_initialScale = 1.0;
};

#endif // SELECTIONBOX_H
