#ifndef NANDGATE_H
#define NANDGATE_H
#include "Gate.h"

class NandGate : public Gate {
public:
    NandGate(QGraphicsItem *parent = nullptr);

protected:
    QPainterPath getPath() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
};
#endif
