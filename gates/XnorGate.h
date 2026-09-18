#ifndef XNORGATE_H
#define XNORGATE_H
#include "Gate.h"

class XnorGate : public Gate {
public:
    XnorGate(QGraphicsItem *parent = nullptr);

protected:
    QPainterPath getPath() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
};
#endif