#ifndef GND_H
#define GND_H

#include "Input.h"

class GND : public Input {
public:
    explicit GND(QGraphicsItem *parent = nullptr);

    LogicState getLogicState() const override { return LogicState::LOW; }

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
};

#endif // GND_H