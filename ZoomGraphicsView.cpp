#include "ZoomGraphicsView.h"
#include "CircuitScene.h"
#include "elements/Switch.h"
#include <QGraphicsItem>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QApplication>
#include <QCursor>
#include <cmath>

ZoomGraphicsView::ZoomGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::NoDrag);
    viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    viewport()->setCursor(Qt::OpenHandCursor);
}

void ZoomGraphicsView::setCircuitScene(CircuitScene *scene)
{
    m_circuitScene = scene;
    applyCursor();
}

void ZoomGraphicsView::setSelectMode(bool active)
{
    m_isSelectMode = active;
    applyCursor();
}

void ZoomGraphicsView::applyCursor()
{
    if (m_isPanning) {
        viewport()->setCursor(Qt::ClosedHandCursor);
    } else if (m_spacePressed) {
        viewport()->setCursor(Qt::OpenHandCursor);
    } else if (m_circuitScene && (m_circuitScene->isPlacementMode() || m_circuitScene->isWireToolActive())) {
        viewport()->setCursor(Qt::CrossCursor);
    } else {
        // If an item has grabbed the mouse (e.g. dragging a resize handle),
        // maintain its cursor during the drag gesture.
        if (m_circuitScene && m_circuitScene->mouseGrabberItem()) {
            QGraphicsItem *grabber = m_circuitScene->mouseGrabberItem();
            if (grabber->hasCursor()) {
                viewport()->setCursor(grabber->cursor());
                return;
            }
            viewport()->setCursor(Qt::ArrowCursor);
            return;
        }

        QPoint localPos = viewport()->mapFromGlobal(QCursor::pos());
        const QList<QGraphicsItem *> itemList = items(localPos);
        QGraphicsItem *cursorItem = nullptr;
        for (QGraphicsItem *item : itemList) {
            if (item->hasCursor()) {
                cursorItem = item;
                break;
            }
        }
        if (cursorItem) {
            viewport()->setCursor(cursorItem->cursor());
        } else if (!itemList.isEmpty()) {
            viewport()->setCursor(Qt::ArrowCursor);
        } else {
            // Hovering over empty canvas
            if (m_isSelectMode) {
                viewport()->setCursor(Qt::CrossCursor);
            } else {
                viewport()->setCursor(Qt::OpenHandCursor);
            }
        }
    }
}

bool ZoomGraphicsView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::NativeGesture) {
        QNativeGestureEvent *ge = static_cast<QNativeGestureEvent *>(event);
        if (ge->gestureType() == Qt::ZoomNativeGesture) {
            double value = ge->value(); // Pinch scale delta
            double factor = 1.0 + value;

            if (m_zoomLevel * factor < kZoomMin) {
                factor = kZoomMin / m_zoomLevel;
            } else if (m_zoomLevel * factor > kZoomMax) {
                factor = kZoomMax / m_zoomLevel;
            }

            if (!qFuzzyCompare(factor, 1.0)) {
                m_zoomLevel *= factor;
                setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
                scale(factor, factor);
            }
            return true;
        }
    }
    return QGraphicsView::viewportEvent(event);
}

void ZoomGraphicsView::wheelEvent(QWheelEvent *event)
{
    bool isControlPressed = (event->modifiers() & Qt::ControlModifier);
    bool hasHorizontalDelta = (event->angleDelta().x() != 0);

    // Touchpad two-finger scroll panning (when Ctrl is not held and horizontal scroll or pixelDelta is present)
    if ((hasHorizontalDelta || event->hasPixelDelta()) && !isControlPressed) {
        QPoint pixelDelta = event->pixelDelta();
        QPoint angleDelta = event->angleDelta();

        int dx = !pixelDelta.isNull() ? pixelDelta.x() : angleDelta.x() / 2;
        int dy = !pixelDelta.isNull() ? pixelDelta.y() : angleDelta.y() / 2;

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - dx);
        verticalScrollBar()->setValue(verticalScrollBar()->value() - dy);
        event->accept();
        return;
    }

    // Zooming (Mouse Wheel or Ctrl + Touchpad scroll)
    double dy = event->angleDelta().y();
    if (dy == 0) {
        dy = event->pixelDelta().y();
    }
    if (qFuzzyIsNull(dy)) {
        event->accept();
        return;
    }

    double factor = std::pow(1.15, dy / 120.0);

    if (m_zoomLevel * factor < kZoomMin) {
        factor = kZoomMin / m_zoomLevel;
    } else if (m_zoomLevel * factor > kZoomMax) {
        factor = kZoomMax / m_zoomLevel;
    }

    if (qFuzzyCompare(factor, 1.0)) {
        event->accept();
        return;
    }

    m_zoomLevel *= factor;

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    scale(factor, factor);

    event->accept();
}

void ZoomGraphicsView::mousePressEvent(QMouseEvent *event)
{
    bool isMiddle = (event->button() == Qt::MiddleButton);
    bool isSpaceLeft = (event->button() == Qt::LeftButton && m_spacePressed);

    // ── Panning: Middle-click or Space + Left-click ──────────────────────────
    if (isMiddle || isSpaceLeft) {
        m_isPanning = true;
        m_panStartPos = event->pos();
        viewport()->setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    // ── Left-click ────────────────────────────────────────────────────────────
    if (event->button() == Qt::LeftButton) {
        bool inPlacement = m_circuitScene && (m_circuitScene->isPlacementMode() || m_circuitScene->isWireToolActive());

        if (inPlacement) {
            setDragMode(QGraphicsView::NoDrag);
            QGraphicsView::mousePressEvent(event);
            applyCursor();
            return;
        }

        QGraphicsItem *hit = itemAt(event->pos());

        if (hit != nullptr) {
            // Clicking directly on an individual element (Gate, Wire, LED bulb, Switch, etc.):
            // Forward directly to scene so user can select, move, or toggle it without needing
            // to click the Select tool button first!
            setDragMode(QGraphicsView::NoDrag);
            QGraphicsView::mousePressEvent(event);
            applyCursor();
            return;
        }

        // Empty canvas clicked:
        if (m_isSelectMode) {
            // Select mode active: dragging draws a rectangular rubberband selection box
            setDragMode(QGraphicsView::RubberBandDrag);
            QGraphicsView::mousePressEvent(event);
            applyCursor();
            return;
        } else {
            // Default mode: deselect previous items if any, then pan canvas
            if (m_circuitScene && !m_circuitScene->selectedItems().isEmpty()) {
                m_circuitScene->clearSelection();
            }
            setDragMode(QGraphicsView::NoDrag);
            m_isPanning = true;
            m_panStartPos = event->pos();
            viewport()->setCursor(Qt::ClosedHandCursor);
            event->accept();
            return;
        }
    }

    QGraphicsView::mousePressEvent(event);
    applyCursor();
}

void ZoomGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPanning) {
        QPoint delta = event->pos() - m_panStartPos;
        m_panStartPos = event->pos();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());

        viewport()->setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
    applyCursor();
}

void ZoomGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPanning) {
        m_isPanning = false;
        applyCursor();
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
    setDragMode(QGraphicsView::NoDrag);
    applyCursor();
}

void ZoomGraphicsView::enterEvent(QEnterEvent *event)
{
    applyCursor();
    QGraphicsView::enterEvent(event);
}

void ZoomGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_spacePressed = true;
        applyCursor();
        event->accept();
        return;
    }
    QGraphicsView::keyPressEvent(event);
}

void ZoomGraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_spacePressed = false;
        applyCursor();
        event->accept();
        return;
    }
    QGraphicsView::keyReleaseEvent(event);
}
