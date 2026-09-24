#include "SelectionBox.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QCursor>
#include <QLineF>
#include <QtMath>

SelectionBox::SelectionBox(QGraphicsItem *targetItem)
    : QGraphicsItem(targetItem), m_targetItem(targetItem) {
    setZValue(1000);
    setAcceptHoverEvents(true);

    if (m_targetItem) {
        m_targetItem->setTransformOriginPoint(m_targetItem->boundingRect().center());
    }
}

QRectF SelectionBox::getHandleRect(const QPointF &center, qreal size) const {
    return QRectF(center.x() - size / 2.0, center.y() - size / 2.0, size, size);
}

QRectF SelectionBox::boundingRect() const {
    if (!m_targetItem)
        return QRectF();

    qreal s = m_targetItem->scale();
    if (s <= 0.001) s = 1.0;

    qreal padding = 6.0 / s;
    qreal handleSize = 8.0 / s;

    QRectF targetRect = m_targetItem->boundingRect();
    QRectF paddedRect = targetRect.adjusted(-padding, -padding, padding, padding);
    return paddedRect.adjusted(-handleSize, -handleSize, handleSize, handleSize);
}

SelectionBox::Handle SelectionBox::hitTestHandle(const QPointF &pos) const {
    if (!m_targetItem)
        return HandleNone;

    qreal s = m_targetItem->scale();
    if (s <= 0.001) s = 1.0;

    qreal padding = 6.0 / s;
    qreal handleSize = 8.0 / s;
    qreal hitRadius = (handleSize / 2.0) + (4.0 / s);

    QRectF targetRect = m_targetItem->boundingRect();
    QRectF paddedRect = targetRect.adjusted(-padding, -padding, padding, padding);

    if (QLineF(pos, paddedRect.topLeft()).length() <= hitRadius)
        return HandleTopLeft;
    if (QLineF(pos, paddedRect.topRight()).length() <= hitRadius)
        return HandleTopRight;
    if (QLineF(pos, paddedRect.bottomLeft()).length() <= hitRadius)
        return HandleBottomLeft;
    if (QLineF(pos, paddedRect.bottomRight()).length() <= hitRadius)
        return HandleBottomRight;

    return HandleNone;
}

void SelectionBox::hoverMoveEvent(QGraphicsSceneHoverEvent *event) {
    Handle handle = hitTestHandle(event->pos());
    switch (handle) {
    case HandleTopLeft:
    case HandleBottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case HandleTopRight:
    case HandleBottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;
    default:
        unsetCursor();
        break;
    }
    QGraphicsItem::hoverMoveEvent(event);
}

void SelectionBox::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    unsetCursor();
    QGraphicsItem::hoverLeaveEvent(event);
}

void SelectionBox::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_targetItem) {
        m_activeHandle = hitTestHandle(event->pos());
        if (m_activeHandle != HandleNone) {
            m_isResizing = true;
            m_initialScale = m_targetItem->scale();
            if (m_initialScale <= 0.001) m_initialScale = 1.0;

            event->accept();
            return;
        }
    }
    m_activeHandle = HandleNone;
    event->ignore(); // Let parent handle moving/selecting!
}

void SelectionBox::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (m_isResizing && m_activeHandle != HandleNone && m_targetItem) {
        QPointF centerScene = m_targetItem->mapToScene(m_targetItem->boundingRect().center());
        QPointF vStart = event->buttonDownScenePos(Qt::LeftButton) - centerScene;
        QPointF vCurr = event->scenePos() - centerScene;

        qreal dot = vCurr.x() * vStart.x() + vCurr.y() * vStart.y();
        qreal vStartLenSq = vStart.x() * vStart.x() + vStart.y() * vStart.y();

        qreal factor = 1.0;
        if (vStartLenSq > 0.001) {
            factor = dot / vStartLenSq;
        }

        // Strictly preserve aspect ratio by scaling uniformly
        qreal newScale = qBound(0.4, m_initialScale * factor, 3.5);
        m_targetItem->setScale(newScale);

        prepareGeometryChange();
        update();
        event->accept();
        return;
    }
    event->ignore();
}

void SelectionBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (m_isResizing) {
        m_isResizing = false;
        m_activeHandle = HandleNone;
        prepareGeometryChange();
        update();
        event->accept();
        return;
    }
    event->ignore();
}

void SelectionBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    if (!m_targetItem)
        return;

    qreal s = m_targetItem->scale();
    if (s <= 0.001) s = 1.0;

    qreal padding = 6.0 / s;
    qreal handleSize = 8.0 / s;
    QRectF targetRect = m_targetItem->boundingRect();
    QRectF paddedRect = targetRect.adjusted(-padding, -padding, padding, padding);

    // 1. Draw dashed selection boundary
    QPen borderPen(QColor(25, 118, 210), 1.5 / s, Qt::DashLine);
    painter->setPen(borderPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(paddedRect);

    // 2. Draw 4 corner handles
    QPen handlePen(QColor(25, 118, 210), 1.5 / s, Qt::SolidLine);
    QBrush handleBrush(Qt::white);
    painter->setPen(handlePen);
    painter->setBrush(handleBrush);

    QRectF rectTL = getHandleRect(paddedRect.topLeft(), handleSize);
    QRectF rectTR = getHandleRect(paddedRect.topRight(), handleSize);
    QRectF rectBL = getHandleRect(paddedRect.bottomLeft(), handleSize);
    QRectF rectBR = getHandleRect(paddedRect.bottomRight(), handleSize);

    painter->drawRect(rectTL);
    painter->drawRect(rectTR);
    painter->drawRect(rectBL);
    painter->drawRect(rectBR);
}
