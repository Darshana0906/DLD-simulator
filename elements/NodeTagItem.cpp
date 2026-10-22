#include "NodeTagItem.h"
#include "../CircuitScene.h"
#include <QPainter>
#include <QFontMetrics>
#include <QPainterPath>
#include <algorithm>

NodeTagItem::NodeTagItem(const QString &text, const QPointF &anchorPos, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_text(text) {
    setPos(anchorPos);
    setAcceptedMouseButtons(Qt::NoButton);
    setZValue(100);

    m_font = QFont("SansSerif", 9, QFont::Bold);
    calculateGeometry();

    updateVisibilityFromScene();
}

void NodeTagItem::updateVisibilityFromScene() {
    if (scene()) {
        if (CircuitScene *cs = dynamic_cast<CircuitScene*>(scene())) {
            setVisible(cs->showTags());
            return;
        }
    }
    if (parentItem() && parentItem()->scene()) {
        if (CircuitScene *cs = dynamic_cast<CircuitScene*>(parentItem()->scene())) {
            setVisible(cs->showTags());
            return;
        }
    }
    setVisible(false);
}

QVariant NodeTagItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemSceneHasChanged && scene()) {
        if (CircuitScene *cs = dynamic_cast<CircuitScene*>(scene())) {
            setVisible(cs->showTags());
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void NodeTagItem::setTagText(const QString &text) {
    prepareGeometryChange();
    m_text = text;
    calculateGeometry();
    update();
}

void NodeTagItem::calculateGeometry() {
    QFontMetrics fm(m_font);
    int textW = fm.horizontalAdvance(m_text);
    qreal boxW = std::max(textW + 12.0, 26.0);
    qreal boxH = 18.0;

    qreal boxX = 6.0;
    qreal boxY = -boxH - 12.0;
    m_boxRect = QRectF(boxX, boxY, boxW, boxH);

    m_boundingRect = QRectF(-2.0, boxY - 2.0, boxX + boxW + 4.0, (-boxY) + 4.0);
}

QRectF NodeTagItem::boundingRect() const {
    return m_boundingRect;
}

void NodeTagItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setRenderHint(QPainter::Antialiasing);

    // 1. Pointer tail
    QPainterPath tail;
    tail.moveTo(m_boxRect.left() + 2.0, m_boxRect.bottom());
    tail.lineTo(m_boxRect.left() + 9.0, m_boxRect.bottom());
    tail.lineTo(0.0, 0.0);
    tail.closeSubpath();

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(99, 102, 241, 200));
    painter->drawPath(tail);

    // 2. Rounded badge box
    painter->setBrush(QColor(255, 255, 255, 245));
    painter->setPen(QPen(QColor(67, 56, 202), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawRoundedRect(m_boxRect, 3.5, 3.5);

    // 3. Label text
    painter->setFont(m_font);
    painter->setPen(QColor(49, 46, 129));
    painter->drawText(m_boxRect, Qt::AlignCenter, m_text);
}
