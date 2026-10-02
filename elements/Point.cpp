#include "Point.h"
#include "../CircuitScene.h"
#include <QPainter>
#include <QCursor>

Point::Point(QGraphicsItem *parent, bool movable, PinType type)
    : QGraphicsItem(parent), m_pinType(type) {
    if (movable)
        setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    setZValue(100); // Pins render above gates
}

Point::~Point() {
}

void Point::setHighlighted(bool highlight) {
    if (m_isHovered != highlight) {
        m_isHovered = highlight;
        update();
    }
}

void Point::setSelectedForWire(bool selected) {
    if (m_isSelectedForWire != selected) {
        m_isSelectedForWire = selected;
        update();
    }
}

void Point::addWire(Wire *wire) {
    if (wire && !m_wires.contains(wire)) {
        m_wires.append(wire);
    }
}

void Point::removeWire(Wire *wire) {
    m_wires.removeAll(wire);
}

QPointF Point::getCenterScenePos() const {
    return mapToScene(QPointF(5, 5));
}

QRectF Point::boundingRect() const {
    // Extended bounding box to accommodate hover glow ring
    return QRectF(-3, -3, 16, 16);
}

void Point::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setRenderHint(QPainter::Antialiasing);

    if (m_isSelectedForWire) {
        // Glowing animated green/blue ring when selected as Start Pin
        painter->setPen(QPen(QColor(76, 175, 80, 200), 2.5));
        painter->setBrush(QColor(76, 175, 80, 100));
        painter->drawEllipse(-2, -2, 14, 14);

        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(46, 125, 50));
        painter->drawEllipse(1, 1, 8, 8);
    } else if (m_isHovered) {
        // Vibrant blue hover ring
        painter->setPen(QPen(QColor(33, 150, 243, 220), 2));
        painter->setBrush(QColor(33, 150, 243, 80));
        painter->drawEllipse(-1, -1, 12, 12);

        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(25, 118, 210));
        painter->drawEllipse(1, 1, 8, 8);
    } else {
        // Standard pin dot
        painter->setPen(QPen(Qt::black, 1));
        painter->setBrush(Qt::darkGray);
        painter->drawEllipse(0, 0, 10, 10);
    }
}

void Point::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    m_isHovered = true;
    setCursor(QCursor(Qt::PointingHandCursor));
    update();
    QGraphicsItem::hoverEnterEvent(event);
}

void Point::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    m_isHovered = false;
    unsetCursor();
    update();
    QGraphicsItem::hoverLeaveEvent(event);
}

void Point::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    CircuitScene *cs = dynamic_cast<CircuitScene *>(scene());
    if (cs && cs->isPlacementMode()) {
        // Notify scene of pin selection
        cs->onPointClicked(this);
        event->accept();
        return;
    }
    QGraphicsItem::mousePressEvent(event);
}