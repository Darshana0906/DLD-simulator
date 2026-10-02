#include "Wire.h"
#include "Point.h"
#include "../overlapping.h"
#include <QPainter>
#include <QPen>
#include <QGraphicsScene>
#include <QLineF>
#include <QtMath>
#include <algorithm>

Wire::Wire(Point *start, Point *end, QGraphicsItem *parent)
    : QGraphicsItem(parent), startPoint(start), endPoint(end) {
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

QVariant Wire::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == QGraphicsItem::ItemPositionChange && scene()) {
        const QPointF delta = parentItem()
            ? parentItem()->mapToScene(value.toPointF()) - parentItem()->mapToScene(pos())
            : value.toPointF() - pos();
        if (!canMoveItem(this, sceneBoundingRect().translated(delta)))
            return pos();
    }
    return QGraphicsItem::itemChange(change, value);
}

QRectF Wire::boundingRect() const {
    return m_cachedBoundingRect;
}

QPainterPath Wire::shape() const {
    QPainterPathStroker stroker;
    stroker.setWidth(10);
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);

    QPainterPath path;
    if (m_pathPoints.size() >= 2) {
        path.moveTo(m_pathPoints.first());
        for (int i = 1; i < m_pathPoints.size(); ++i) {
            path.lineTo(m_pathPoints[i]);
        }
    }
    return stroker.createStroke(path);
}

void Wire::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    if (m_pathPoints.size() < 2)
        return;

    painter->setRenderHint(QPainter::Antialiasing);

    QColor wireColor = isSelected() ? QColor(33, 150, 243) : QColor(33, 33, 33);
    qreal lineWidth = isSelected() ? 3.0 : 2.0;

    if (m_isPreview) {
        wireColor = QColor(76, 175, 80);
        QPen pen(wireColor, 2.0, Qt::DashLine);
        painter->setPen(pen);
    } else {
        QPen pen(wireColor, lineWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
        painter->setPen(pen);
    }

    // Collect scene horizontal segments from OTHER wires
    QList<QLineF> otherHrzSegments;
    if (scene() && !m_isPreview) {
        for (QGraphicsItem *item : scene()->items()) {
            Wire *otherWire = dynamic_cast<Wire*>(item);
            if (otherWire && otherWire != this && !otherWire->m_isPreview) {
                QVector<QPointF> otherSegs = otherWire->getSegments();
                for (int i = 0; i < otherSegs.size() - 1; ++i) {
                    QPointF pA = otherWire->mapToScene(otherSegs[i]);
                    QPointF pB = otherWire->mapToScene(otherSegs[i + 1]);
                    if (std::abs(pA.y() - pB.y()) < 0.5) { // Horizontal segment
                        otherHrzSegments.append(QLineF(pA, pB));
                    }
                }
            }
        }
    }

    QPainterPath path;
    path.moveTo(m_pathPoints.first());

    const qreal r = 6.0; // Radius of semicircular bridge arc

    for (int i = 0; i < m_pathPoints.size() - 1; ++i) {
        QPointF pStart = m_pathPoints[i];
        QPointF pEnd = m_pathPoints[i + 1];

        QPointF pStartScene = mapToScene(pStart);
        QPointF pEndScene = mapToScene(pEnd);

        bool isVertical = std::abs(pStart.x() - pEnd.x()) < 0.5;

        if (isVertical && !otherHrzSegments.isEmpty()) {
            qreal vx = pStart.x();
            qreal sceneVx = pStartScene.x();
            qreal yMinScene = std::min(pStartScene.y(), pEndScene.y());
            qreal yMaxScene = std::max(pStartScene.y(), pEndScene.y());

            // Collect all horizontal crossing Y coordinates along this vertical segment
            QVector<qreal> crossings;
            for (const QLineF &hrz : otherHrzSegments) {
                qreal hxMin = std::min(hrz.x1(), hrz.x2());
                qreal hxMax = std::max(hrz.x1(), hrz.x2());
                qreal hy = hrz.y1();

                // Check if crossing occurs
                bool xOverlap = (sceneVx >= hxMin - 1.0) && (sceneVx <= hxMax + 1.0);
                bool yOverlap = (hy >= yMinScene - 1.0) && (hy <= yMaxScene + 1.0);
                if (xOverlap && yOverlap) {
                    QPointF localCross = mapFromScene(QPointF(sceneVx, hy));
                    crossings.append(localCross.y());
                }
            }

            if (!crossings.isEmpty()) {
                // Sort crossings along the direction of travel
                bool goingDown = (pEnd.y() > pStart.y());
                std::sort(crossings.begin(), crossings.end(), [goingDown](qreal a, qreal b) {
                    return goingDown ? (a < b) : (a > b);
                });

                for (qreal yCross : crossings) {
                    if (goingDown) {
                        path.lineTo(vx, yCross - r);
                        path.arcTo(QRectF(vx - r, yCross - r, 2 * r, 2 * r), 90, -180);
                    } else {
                        path.lineTo(vx, yCross + r);
                        path.arcTo(QRectF(vx - r, yCross - r, 2 * r, 2 * r), -90, -180);
                    }
                }
            }
        }

        path.lineTo(pEnd);
    }

    painter->drawLine(start, end);
}
