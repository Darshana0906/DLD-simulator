#ifndef ORGATE_H
#define ORGATE_H
#include "Gate.h"

class OrGate : public Gate {
public:
    OrGate(QGraphicsItem *parent = nullptr);

protected:
    QPainterPath getPath() const override;

    void paint(QPainter *painter,
                const QStyleOptionGraphicsItem *option,
                QWidget *widget = nullptr) override;
};
#endif