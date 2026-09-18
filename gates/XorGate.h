#ifndef XORGATE_H
#define XORGATE_H
#include "Gate.h"

class XorGate : public Gate {
public:
    XorGate(QGraphicsItem *parent = nullptr);

protected:
    QPainterPath getPath() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
};
#endif