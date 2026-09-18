#ifndef SWITCH_H
#define SWITCH_H
#include "Input.h"
#include <QGraphicsSceneMouseEvent>

class Switch : public Input {
public:
    Switch(QGraphicsItem *parent = nullptr);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif