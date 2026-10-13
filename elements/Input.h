#ifndef INPUT_H
#define INPUT_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include "../logic/LogicTypes.h"

class Point;
class Input : public QGraphicsItem {
public:
    Input(bool value = false, QGraphicsItem *parent = nullptr);
    virtual ~Input() = default;

    bool getValue() const;
    virtual void setValue(bool value);
    Point* getOutputPoint() const;

    virtual LogicState getLogicState() const;

protected:
    bool value;
    Point *outputPoint;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

#endif
