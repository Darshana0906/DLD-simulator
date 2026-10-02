#ifndef CONST1_H

#define CONST1_H
#include "Input.h"
class Const1 : public Input {
protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
public:
    Const1(QGraphicsItem *parent = nullptr);
};

#endif
