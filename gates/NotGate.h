#ifndef NOTGATE_H
#define NOTGATE_H

#include "Gate.h"

class NotGate : public Gate
{
public:
    NotGate(QGraphicsItem *parent = nullptr);

protected:
    QPainterPath getPath() const override;
};

#endif