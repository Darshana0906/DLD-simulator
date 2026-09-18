#ifndef ANDGATE_H
#define ANDGATE_H
#include "Gate.h"

class AndGate : public Gate {
public:
    AndGate(QGraphicsItem *parent = nullptr);

protected:
    QPainterPath getPath() const override;

    void paint(QPainter *painter,
                const QStyleOptionGraphicsItem *option,
                QWidget *widget = nullptr) override;
};
#endif