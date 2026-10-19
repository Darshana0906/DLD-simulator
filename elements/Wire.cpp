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
    setZValue(10); // Wires render below pins, above background

    if (startPoint) {
        startPoint->addWire(this);
        m_savedStartPos = startPoint->getCenterScenePos();
        m_savedStartPinType = startPoint->getPinType();
        m_hasStartPos = true;
    }
    if (endPoint) {
        endPoint->addWire(this);
        m_savedEndPos = endPoint->getCenterScenePos();
        m_savedEndPinType = endPoint->getPinType();
        m_hasEndPos = true;
        m_isPreview = false;
    } else {
        m_isPreview = true;
    }

    updatePath();
}

Wire::~Wire() {
    if (startPoint) {
        startPoint->removeWire(this);
        startPoint = nullptr;
    }
    if (endPoint) {
        endPoint->removeWire(this);
        endPoint = nullptr;
    }
}

void Wire::detachPoint(Point *point) {
    if (!point) return;

    if (startPoint == point) {
        // Save last known position and pin type before nulling
        m_savedStartPos = startPoint->getCenterScenePos();
        m_savedStartPinType = startPoint->getPinType();
        m_hasStartPos = true;
        startPoint = nullptr;
    }
    if (endPoint == point) {
        m_savedEndPos = endPoint->getCenterScenePos();
        m_savedEndPinType = endPoint->getPinType();
        m_hasEndPos = true;
        endPoint = nullptr;
    }

    // Broken connection becomes undefined logic state
    setLogicValue(-1);

    // Recalculate path using saved positions
    updatePath();
}

void Wire::setEndPoint(Point *end) {
    if (endPoint) {
        endPoint->removeWire(this);
    }
    endPoint = end;
    if (endPoint) {
        endPoint->addWire(this);
        m_savedEndPos = endPoint->getCenterScenePos();
        m_savedEndPinType = endPoint->getPinType();
        m_hasEndPos = true;
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

#include <vector>
#include <queue>
#include <limits>

QVariant Wire::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemSceneHasChanged && scene()) {
        updatePath();
    }
    return QGraphicsItem::itemChange(change, value);
}

static bool isSegmentBlocked(const QPointF &p1, const QPointF &p2, const QVector<QRectF> &obstacles) {
    const qreal eps = 0.5;
    for (const QRectF &obs : obstacles) {
        if (std::abs(p1.y() - p2.y()) < eps) {
            // Horizontal segment
            qreal y = p1.y();
            if (y > obs.top() + eps && y < obs.bottom() - eps) {
                qreal minX = std::min(p1.x(), p2.x());
                qreal maxX = std::max(p1.x(), p2.x());
                if (maxX > obs.left() + eps && minX < obs.right() - eps) {
                    return true;
                }
            }
        } else if (std::abs(p1.x() - p2.x()) < eps) {
            // Vertical segment
            qreal x = p1.x();
            if (x > obs.left() + eps && x < obs.right() - eps) {
                qreal minY = std::min(p1.y(), p2.y());
                qreal maxY = std::max(p1.y(), p2.y());
                if (maxY > obs.top() + eps && minY < obs.bottom() - eps) {
                    return true;
                }
            }
        } else {
            // General segment check
            QLineF seg(p1, p2);
            QLineF top(obs.topLeft(), obs.topRight());
            QLineF bottom(obs.bottomLeft(), obs.bottomRight());
            QLineF left(obs.topLeft(), obs.bottomLeft());
            QLineF right(obs.topRight(), obs.bottomRight());
            QPointF dummy;
            if (obs.contains(p1) || obs.contains(p2) ||
                seg.intersects(top, &dummy) == QLineF::BoundedIntersection ||
                seg.intersects(bottom, &dummy) == QLineF::BoundedIntersection ||
                seg.intersects(left, &dummy) == QLineF::BoundedIntersection ||
                seg.intersects(right, &dummy) == QLineF::BoundedIntersection) {
                return true;
            }
        }
    }
    return false;
}

static bool isPathBlocked(const QVector<QPointF> &path, const QVector<QRectF> &obstacles) {
    for (int i = 0; i + 1 < path.size(); ++i) {
        if (isSegmentBlocked(path[i], path[i + 1], obstacles)) {
            return true;
        }
    }
    return false;
}

static QVector<QPointF> simplifyCollinear(const QVector<QPointF> &pts) {
    QVector<QPointF> res;
    for (int i = 0; i < pts.size(); ++i) {
        if (res.size() >= 2) {
            QPointF pA = res[res.size() - 2];
            QPointF pB = res.last();
            QPointF pC = pts[i];
            bool sameH = (std::abs(pA.y() - pB.y()) < 0.5 && std::abs(pB.y() - pC.y()) < 0.5);
            bool sameV = (std::abs(pA.x() - pB.x()) < 0.5 && std::abs(pB.x() - pC.x()) < 0.5);
            if (sameH || sameV) {
                res.last() = pC;
                continue;
            }
        }
        res.append(pts[i]);
    }
    return res;
}

void Wire::calculateManhattanPath() {
    m_pathPoints.clear();

    // Determine scene positions — use saved positions for destroyed endpoints
    QPointF startScene, endScene;
    PinType startType, endType;

    if (startPoint) {
        startScene = startPoint->getCenterScenePos();
        startType = startPoint->getPinType();
        m_savedStartPos = startScene;
        m_savedStartPinType = startType;
        m_hasStartPos = true;
    } else if (m_hasStartPos) {
        startScene = m_savedStartPos;
        startType = m_savedStartPinType;
    } else {
        m_cachedBoundingRect = QRectF();
        return;
    }

    if (endPoint) {
        endScene = endPoint->getCenterScenePos();
        endType = endPoint->getPinType();
        m_savedEndPos = endScene;
        m_savedEndPinType = endType;
        m_hasEndPos = true;
    } else if (m_isPreview) {
        endScene = m_previewPos;
        endType = PinType::Input;
    } else if (m_hasEndPos) {
        endScene = m_savedEndPos;
        endType = m_savedEndPinType;
    } else {
        m_cachedBoundingRect = QRectF();
        return;
    }

    // Input pins extend LEFT (-25px); Output/Standalone pins extend RIGHT (+25px)
    qreal startStubDx = (startType == PinType::Input) ? -25.0 : 25.0;
    qreal endStubDx = (endType == PinType::Output) ? 25.0 : -25.0;

    QPointF stub1(startScene.x() + startStubDx, startScene.y());
    QPointF stub2(endScene.x() + endStubDx, endScene.y());

    // Collect solid component body obstacles
    QVector<QRectF> obstacles;
    QGraphicsScene *sc = scene();
    if (!sc && startPoint) sc = startPoint->scene();
    if (!sc && endPoint) sc = endPoint->scene();

    QSet<QGraphicsItem *> checkedTops;
    if (sc) {
        for (QGraphicsItem *item : sc->items()) {
            if (!item || item == this) continue;
            if (!isSolidComponent(item)) continue;

            QGraphicsItem *solidTop = item->topLevelItem();
            if (!solidTop || checkedTops.contains(solidTop)) continue;
            checkedTops.insert(solidTop);

            QRectF body = getComponentBodyRect(solidTop);
            if (body.isValid() && !body.isEmpty()) {
                obstacles.append(body.adjusted(-2.0, -6.0, 2.0, 6.0));
            }
        }
    }

    // Candidate default paths
    QVector<QPointF> chosenPath;

    // Check simple paths first
    if (qFuzzyCompare(stub1.y(), stub2.y())) {
        bool forwardH = (startStubDx > 0 && stub2.x() >= stub1.x()) || (startStubDx < 0 && stub2.x() <= stub1.x());
        if (forwardH) {
            QVector<QPointF> cand = {stub1, stub2};
            if (!isPathBlocked(cand, obstacles)) {
                chosenPath = cand;
            }
        }
    }

    if (chosenPath.isEmpty()) {
        if (startStubDx > 0 && endStubDx < 0 && stub1.x() < stub2.x()) {
            // Standard Output -> Input left-to-right flow
            qreal midX = (stub1.x() + stub2.x()) / 2.0;
            QVector<QPointF> cand = {stub1, QPointF(midX, stub1.y()), QPointF(midX, stub2.y()), stub2};
            if (!isPathBlocked(cand, obstacles)) {
                chosenPath = cand;
            }
        } else if (startStubDx < 0 && endStubDx < 0 && stub1.x() < stub2.x()) {
            // Input -> Input flow
            qreal midX = std::min(stub1.x(), stub2.x()) - 20.0;
            QVector<QPointF> cand = {stub1, QPointF(midX, stub1.y()), QPointF(midX, stub2.y()), stub2};
            if (!isPathBlocked(cand, obstacles)) {
                chosenPath = cand;
            }
        }
    }

    // If direct paths are blocked by an obstacle, run Hanan Grid A* Router
    if (chosenPath.isEmpty() && !obstacles.isEmpty()) {
        std::vector<qreal> xs;
        std::vector<qreal> ys;

        xs.push_back(stub1.x());
        xs.push_back(stub2.x());
        ys.push_back(stub1.y());
        ys.push_back(stub2.y());

        qreal minX = std::min(stub1.x(), stub2.x());
        qreal maxX = std::max(stub1.x(), stub2.x());
        qreal minY = std::min(stub1.y(), stub2.y());
        qreal maxY = std::max(stub1.y(), stub2.y());

        for (const QRectF &obs : obstacles) {
            xs.push_back(obs.left());
            xs.push_back(obs.right());
            xs.push_back(obs.left() - 20.0);
            xs.push_back(obs.right() + 20.0);
            ys.push_back(obs.top());
            ys.push_back(obs.bottom());
            ys.push_back(obs.top() - 20.0);
            ys.push_back(obs.bottom() + 20.0);

            minX = std::min(minX, obs.left());
            maxX = std::max(maxX, obs.right());
            minY = std::min(minY, obs.top());
            maxY = std::max(maxY, obs.bottom());
        }

        // Add bounding escape channels
        xs.push_back(minX - 40.0);
        xs.push_back(maxX + 40.0);
        ys.push_back(minY - 40.0);
        ys.push_back(maxY + 40.0);

        std::sort(xs.begin(), xs.end());
        xs.erase(std::unique(xs.begin(), xs.end(), [](qreal a, qreal b) {
            return std::abs(a - b) < 1.0;
        }), xs.end());

        std::sort(ys.begin(), ys.end());
        ys.erase(std::unique(ys.begin(), ys.end(), [](qreal a, qreal b) {
            return std::abs(a - b) < 1.0;
        }), ys.end());

        int W = static_cast<int>(xs.size());
        int H = static_cast<int>(ys.size());

        auto findClosestIdx = [](const std::vector<qreal> &vec, qreal val) -> int {
            int bestIdx = 0;
            qreal bestDiff = std::abs(vec[0] - val);
            for (int i = 1; i < static_cast<int>(vec.size()); ++i) {
                qreal diff = std::abs(vec[i] - val);
                if (diff < bestDiff) {
                    bestDiff = diff;
                    bestIdx = i;
                }
            }
            return bestIdx;
        };

        int startX = findClosestIdx(xs, stub1.x());
        int startY = findClosestIdx(ys, stub1.y());
        int goalX = findClosestIdx(xs, stub2.x());
        int goalY = findClosestIdx(ys, stub2.y());

        struct NodeState {
            qreal fCost;
            qreal gCost;
            int xi;
            int yi;
            int dir; // 0: None, 1: H, 2: V

            bool operator>(const NodeState &other) const {
                return fCost > other.fCost;
            }
        };

        int totalStates = W * H * 3;
        std::vector<qreal> gScore(totalStates, std::numeric_limits<qreal>::infinity());
        std::vector<int> cameFrom(totalStates, -1);

        auto stateIndex = [W](int xi, int yi, int dir) -> int {
            return (yi * W + xi) * 3 + dir;
        };

        std::priority_queue<NodeState, std::vector<NodeState>, std::greater<NodeState>> openSet;

        gScore[stateIndex(startX, startY, 0)] = 0.0;
        qreal h0 = std::abs(xs[startX] - xs[goalX]) + std::abs(ys[startY] - ys[goalY]);
        openSet.push({h0, 0.0, startX, startY, 0});

        int finalEndState = -1;
        const int dxs[4] = {-1, 1, 0, 0};
        const int dys[4] = {0, 0, -1, 1};
        const int dirs[4] = {1, 1, 2, 2};

        while (!openSet.empty()) {
            NodeState curr = openSet.top();
            openSet.pop();

            int currIdx = stateIndex(curr.xi, curr.yi, curr.dir);
            if (curr.gCost > gScore[currIdx]) {
                continue;
            }

            if (curr.xi == goalX && curr.yi == goalY) {
                finalEndState = currIdx;
                break;
            }

            QPointF pCurr(xs[curr.xi], ys[curr.yi]);

            for (int d = 0; d < 4; ++d) {
                int nXi = curr.xi + dxs[d];
                int nYi = curr.yi + dys[d];
                int nDir = dirs[d];

                if (nXi < 0 || nXi >= W || nYi < 0 || nYi >= H) {
                    continue;
                }

                QPointF pNext(xs[nXi], ys[nYi]);
                if (isSegmentBlocked(pCurr, pNext, obstacles)) {
                    continue;
                }

                qreal stepDist = std::abs(xs[nXi] - xs[curr.xi]) + std::abs(ys[nYi] - ys[curr.yi]);
                qreal turnPenalty = (curr.dir != 0 && curr.dir != nDir) ? 40.0 : 0.0;
                qreal tentativeG = curr.gCost + stepDist + turnPenalty;

                int nIdx = stateIndex(nXi, nYi, nDir);
                if (tentativeG < gScore[nIdx]) {
                    gScore[nIdx] = tentativeG;
                    cameFrom[nIdx] = currIdx;
                    qreal h = std::abs(xs[nXi] - xs[goalX]) + std::abs(ys[nYi] - ys[goalY]);
                    openSet.push({tentativeG + h, tentativeG, nXi, nYi, nDir});
                }
            }
        }

        if (finalEndState != -1) {
            int trace = finalEndState;
            while (trace != -1) {
                int rem = trace / 3;
                int yi = rem / W;
                int xi = rem % W;
                chosenPath.append(QPointF(xs[xi], ys[yi]));
                trace = cameFrom[trace];
            }
            std::reverse(chosenPath.begin(), chosenPath.end());
        }
    }

    // Default fallback if pathfinding was completely boxed in
    if (chosenPath.isEmpty()) {
        qreal midX = (stub1.x() + stub2.x()) / 2.0;
        chosenPath = {stub1, QPointF(midX, stub1.y()), QPointF(midX, stub2.y()), stub2};
    }

    // Assemble final route
    if (chosenPath.isEmpty() || chosenPath.first() != stub1) {
        chosenPath.prepend(stub1);
    }
    if (chosenPath.last() != stub2) {
        chosenPath.append(stub2);
    }

    chosenPath = simplifyCollinear(chosenPath);
    chosenPath.prepend(startScene);
    chosenPath.append(endScene);
    chosenPath = simplifyCollinear(chosenPath);

    // Convert from scene coordinates to wire local item coordinates
    for (const QPointF &ptScene : chosenPath) {
        m_pathPoints.append(mapFromScene(ptScene));
    }

    // Compute bounding rect enclosing all path segments
    if (m_pathPoints.isEmpty()) {
        m_cachedBoundingRect = QRectF();
        return;
    }

    qreal minLocalX = m_pathPoints.first().x();
    qreal maxLocalX = minLocalX;
    qreal minLocalY = m_pathPoints.first().y();
    qreal maxLocalY = minLocalY;

    for (const QPointF &pt : m_pathPoints) {
        minLocalX = std::min(minLocalX, pt.x());
        maxLocalX = std::max(maxLocalX, pt.x());
        minLocalY = std::min(minLocalY, pt.y());
        maxLocalY = std::max(maxLocalY, pt.y());
    }

    const qreal margin = 16.0;
    m_cachedBoundingRect = QRectF(minLocalX, minLocalY, maxLocalX - minLocalX, maxLocalY - minLocalY)
                               .adjusted(-margin, -margin, margin, margin);
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

    QColor wireColor = QColor(66, 66, 66);
    qreal lineWidth = 2.0;

    if (isSelected()) {
        wireColor = QColor(33, 150, 243); // Blue for selection
        lineWidth = 3.0;
    } else if (m_logicValue == 1) {
        wireColor = QColor(0, 200, 83);   // Bright green for logic HIGH
        lineWidth = 2.5;
    } else if (m_logicValue == 0) {
        wireColor = QColor(38, 50, 56);   // Dark slate for logic LOW
        lineWidth = 2.0;
    }

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
            const qreal endpointTol = 3.0;
            for (const QLineF &hrz : otherHrzSegments) {
                qreal hxMin = std::min(hrz.x1(), hrz.x2());
                qreal hxMax = std::max(hrz.x1(), hrz.x2());
                qreal hy = hrz.y1();

                // Crossing only occurs strictly in the interior of both segments
                bool xInside = (sceneVx > hxMin + endpointTol) && (sceneVx < hxMax - endpointTol);
                bool yInside = (hy > yMinScene + endpointTol) && (hy < yMaxScene - endpointTol);
                if (xInside && yInside) {
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
