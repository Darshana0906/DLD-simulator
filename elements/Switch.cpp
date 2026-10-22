#include "Switch.h"
#include "Point.h"
#include "CircuitScene.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QPen>
#include <QFont>

Switch::Switch(QGraphicsItem *parent) : Input(false, parent) {
    if (outputPoint) {
        outputPoint->setPos(50, 20);
    }
}

void Switch::setValue(bool v) {
    bool changed = (value != v);
    Input::setValue(v);
    if (changed && m_onToggle) {
        m_onToggle(this, v);
    }
}

void Switch::setOnToggleCallback(ToggleCallback callback) {
    m_onToggle = std::move(callback);
}

Switch::ToggleCallback Switch::getOnToggleCallback() const {
    return m_onToggle;
}

QRectF Switch::boundingRect() const {
    return QRectF(0, 0, 65, 50);
}

void Switch::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
    painter->setPen(pen);

    if (value) {
        painter->setBrush(Qt::green);
    } else {
        painter->setBrush(Qt::white);
    }

    QRectF circleRect(10, 10, 30, 30);
    painter->drawEllipse(circleRect);

    // Terminal lead wire connected to output pin (pin center is at 55, 25)
    painter->drawLine(40, 25, 55, 25);

    // Draw "0/1" label inside the circle matching switchip.png
    QFont font = painter->font();
    font.setBold(true);
    font.setPixelSize(11);
    painter->setFont(font);
    painter->setPen(Qt::black);
    painter->drawText(circleRect, Qt::AlignCenter, "0/1");
}

void Switch::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // During placement mode the scene needs to receive this click to place
    // a new element — do not consume it or toggle state here.
    CircuitScene *cs = dynamic_cast<CircuitScene *>(scene());
    if (cs && cs->isPlacementMode()) {
        event->ignore(); // let the scene's mousePressEvent handle it
        return;
    }
    setValue(!getValue());
    if (cs) {
        emit cs->circuitModified();
    }
    QGraphicsItem::mousePressEvent(event);
}