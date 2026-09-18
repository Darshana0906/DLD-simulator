#ifndef NORGATE_H
#define NORGATE_H
#include "Gate.h"

class NorGate : public Gate {
public:
    NorGate(QGraphicsItem *parent = nullptr);

protected:
    QPainterPath getPath() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
};
#endif