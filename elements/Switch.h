#ifndef SWITCH_H
#define SWITCH_H
#include "Input.h"
#include <QGraphicsSceneMouseEvent>

class Switch : public Input {
public:
    explicit Switch(QGraphicsItem *parent = nullptr);

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // SWITCH_H