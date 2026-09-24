#include "ZoomGraphicsView.h"
#include "CircuitScene.h"
#include <QWheelEvent>
#include <QMouseEvent>

ZoomGraphicsView::ZoomGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setTransformationAnchor(QGraphicsView::NoAnchor);
}

void ZoomGraphicsView::setCircuitScene(CircuitScene *scene)
{
    m_circuitScene = scene;
}

// Single source of truth for the cursor: Cross during placement, Arrow otherwise.
// Call this whenever placement mode changes, or after any event that might
// have overridden the cursor (item hover, etc.).
void ZoomGraphicsView::applyCursor()
{
    if (m_circuitScene && m_circuitScene->isPlacementMode()) {
        viewport()->setCursor(Qt::CrossCursor);
    } else {
        viewport()->setCursor(Qt::ArrowCursor);
    }
}

void ZoomGraphicsView::wheelEvent(QWheelEvent *event)
{
    QPointF oldPos = mapToScene(event->position().toPoint());

    double factor;

    if (event->angleDelta().y() > 0)
        factor = 1.2;
    else
        factor = 1.0 / 1.2;

    scale(factor, factor);

    QPointF newPos = mapToScene(event->position().toPoint());

    QPointF delta = newPos - oldPos;

    translate(delta.x(), delta.y());

    event->accept();
}

void ZoomGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    // Re-enforce the correct cursor AFTER the base class call.
    // Qt's item hover machinery (ItemIsMovable, etc.) runs inside the base
    // call and may override the viewport cursor — calling applyCursor() here
    // ensures our intended cursor always wins.
    applyCursor();
}

void ZoomGraphicsView::enterEvent(QEnterEvent *event)
{
    // Immediately apply the correct cursor when the mouse enters the canvas,
    // before any move event fires.
    applyCursor();
    QGraphicsView::enterEvent(event);
}