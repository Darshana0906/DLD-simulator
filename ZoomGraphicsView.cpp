#include "ZoomGraphicsView.h"
#include <QWheelEvent>

ZoomGraphicsView::ZoomGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setTransformationAnchor(QGraphicsView::NoAnchor);
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