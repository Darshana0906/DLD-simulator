#ifndef NOTGATE_H
#define NOTGATE_H
#include "Gate.h"

class NotGate : public Gate {
public:
    NotGate(QGraphicsItem *parent = nullptr);

protected:
    QPainterPath getPath() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
};
#endif