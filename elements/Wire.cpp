#include "Wire.h"
#include "Point.h"
#include <QPainter>
#include <QPen>
#include <QGraphicsScene>
#include <QLineF>
#include <QtMath>
#include <algorithm>

Wire::Wire(Point *start, Point *end, QGraphicsItem *parent)
    : QGraphicsItem(parent), startPoint(start), endPoint(end) {
    setFlag(QGraphicsItem::ItemIsSelectable);
    setZValue(10); // Wires render below pins, above background

    if (startPoint) {
        startPoint->addWire(this);
    }
    if (endPoint) {
        endPoint->addWire(this);
        m_isPreview = false;
    } else {
        m_isPreview = true;
    }

    updatePath();
}

Wire::~Wire() {
    if (startPoint) {
        startPoint->removeWire(this);
    }
    if (endPoint) {
        endPoint->removeWire(this);
    }
}

void Wire::setEndPoint(Point *end) {
    if (endPoint) {
        endPoint->removeWire(this);
    }
    endPoint = end;
    if (endPoint) {
        endPoint->addWire(this);
        m_isPreview = false;
    }
    updatePath();
}

void Wire::setPreviewPos(const QPointF &pos) {
    m_previewPos = pos;
    m_isPreview = true;
    updatePath();
}

void Wire::updatePath() {
    prepareGeometryChange();
    calculateManhattanPath();
    update();

    // Trigger update on all scene wires so crossing arc hops refresh immediately
    if (scene() && !m_isPreview) {
        for (QGraphicsItem *item : scene()->items()) {
            if (Wire *otherWire = dynamic_cast<Wire*>(item)) {
                if (otherWire != this && !otherWire->m_isPreview) {
                    otherWire->update();
                }
            }
        }
    }
}

void Wire::calculateManhattanPath() {
    m_pathPoints.clear();

    if (!startPoint) {
        m_cachedBoundingRect = QRectF();
        return;
    }

    QPointF startScene = startPoint->getCenterScenePos();
    QPointF endScene = (endPoint != nullptr) ? endPoint->getCenterScenePos() : m_previewPos;

    // Convert scene coordinates to local item coordinates
    QPointF p1 = mapFromScene(startScene);
    QPointF p2 = mapFromScene(endScene);

    m_pathPoints.append(p1);

    // Determine stub directions based on pin types
    PinType startType = startPoint->getPinType();
    PinType endType = (endPoint != nullptr) ? endPoint->getPinType() : PinType::Input;

    // Input pins extend LEFT (-25px); Output/Standalone pins extend RIGHT (+25px)
    qreal startStubDx = (startType == PinType::Input) ? -25.0 : 25.0;
    qreal endStubDx = (endType == PinType::Output) ? 25.0 : -25.0;

    QPointF stub1(p1.x() + startStubDx, p1.y());
    QPointF stub2(p2.x() + endStubDx, p2.y());

    m_pathPoints.append(stub1);

    // Route between stub1 and stub2 orthogonally
    if (qFuzzyCompare(stub1.y(), stub2.y())) {
        // Same horizontal line
        m_pathPoints.append(stub2);
    } else if (startStubDx > 0 && endStubDx < 0 && stub1.x() < stub2.x()) {
        // Standard Output -> Input left-to-right flow
        qreal midX = (stub1.x() + stub2.x()) / 2.0;
        m_pathPoints.append(QPointF(midX, stub1.y()));
        m_pathPoints.append(QPointF(midX, stub2.y()));
        m_pathPoints.append(stub2);
    } else if (startStubDx < 0 && endStubDx < 0 && stub1.x() < stub2.x()) {
        // Input -> Input flow
        qreal midX = std::min(stub1.x(), stub2.x());
        m_pathPoints.append(QPointF(midX, stub1.y()));
        m_pathPoints.append(QPointF(midX, stub2.y()));
        m_pathPoints.append(stub2);
    } else {
        // Backward or overlapping routing: route via intermediate midY offset
        qreal midY = (stub1.y() + stub2.y()) / 2.0;
        if (std::abs(stub1.y() - stub2.y()) < 40.0) {
            midY = std::min(stub1.y(), stub2.y()) - 35.0;
        }

        m_pathPoints.append(QPointF(stub1.x(), midY));
        m_pathPoints.append(QPointF(stub2.x(), midY));
        m_pathPoints.append(stub2);
    }

    m_pathPoints.append(p2);

    // Compute bounding rect enclosing all path segments
    if (m_pathPoints.isEmpty()) {
        m_cachedBoundingRect = QRectF();
        return;
    }

    qreal minX = m_pathPoints.first().x();
    qreal maxX = minX;
    qreal minY = m_pathPoints.first().y();
    qreal maxY = minY;

    for (const QPointF &pt : m_pathPoints) {
        minX = std::min(minX, pt.x());
        maxX = std::max(maxX, pt.x());
        minY = std::min(minY, pt.y());
        maxY = std::max(maxY, pt.y());
    }

    const qreal margin = 16.0;
    m_cachedBoundingRect = QRectF(minX, minY, maxX - minX, maxY - minY).adjusted(-margin, -margin, margin, margin);
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

    painter->drawPath(path);
}