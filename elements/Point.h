#ifndef POINT_H
#define POINT_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>

class Wire;

enum class PinType {
    Input,
    Output,
    Standalone
};

class Point : public QGraphicsItem {
public:
    explicit Point(QGraphicsItem *parent = nullptr, bool movable = true, PinType type = PinType::Standalone);
    ~Point();

    PinType getPinType() const { return m_pinType; }
    void setPinType(PinType type) { m_pinType = type; }

    bool isHovered() const { return m_isHovered; }
    void setHighlighted(bool highlight);

    bool isSelectedForWire() const { return m_isSelectedForWire; }
    void setSelectedForWire(bool selected);

    void addWire(Wire *wire);
    void removeWire(Wire *wire);
    const QList<Wire*>& getWires() const { return m_wires; }

    QPointF getCenterScenePos() const;

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    bool m_isHovered = false;
    bool m_isSelectedForWire = false;
    PinType m_pinType = PinType::Standalone;
    QList<Wire*> m_wires;
};

#endif // POINT_H