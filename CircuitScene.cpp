#include "CircuitScene.h"
#include <QPainter>

CircuitScene::CircuitScene(QObject *parent)
    : QGraphicsScene(parent), m_placementMode(false) {
    setBackgroundBrush(QColor(248, 249, 250));
}

void CircuitScene::setPlacementMode(bool active) {
    m_placementMode = active;
}

bool CircuitScene::isPlacementMode() const {
    return m_placementMode;
}

void CircuitScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (m_placementMode) {
        if (event->button() == Qt::LeftButton) {
            emit sceneClicked(event->scenePos());
            event->accept();
            return;
        } else if (event->button() == Qt::RightButton) {
            emit placementCancelled();
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
    QGraphicsScene::keyPressEvent(event);
}

void CircuitScene::drawBackground(QPainter *painter, const QRectF &rect) {
    QGraphicsScene::drawBackground(painter, rect);

    // Draw subtle grid dots every 20 pixels
    const int gridSize = 20;
    qreal left = int(rect.left()) - (int(rect.left()) % gridSize);
    qreal top = int(rect.top()) - (int(rect.top()) % gridSize);

    painter->setPen(QPen(QColor(210, 215, 222), 1));
    for (qreal x = left; x < rect.right(); x += gridSize) {
        for (qreal y = top; y < rect.bottom(); y += gridSize) {
            painter->drawPoint(QPointF(x, y));
        }
    }
}
