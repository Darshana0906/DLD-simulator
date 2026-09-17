#ifndef NANDGATE_H
#define NANDGATE_H

#include "Gate.h"

class NandGate : public Gate
{
public:
    NandGate(QGraphicsItem *parent = nullptr);

protected:
    QPainterPath getPath() const override;
};

#endif
