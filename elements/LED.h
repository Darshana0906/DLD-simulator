#ifndef LED_H
#define LED_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include "../logic/LogicTypes.h"

class Point;
class QPainter;

class LED : public QGraphicsItem {
public:
    explicit LED(QGraphicsItem *parent = nullptr);
    Point* getInputPoint() const;

    // Logic State: -1 = Undefined / Unstimulated (shows '?'), 0 = Off (white), 1 = On (glows yellow)
    int getLogicValue() const;
    void setLogicValue(int val);

    LogicState getLogicState() const;
    void setLogicState(LogicState s);

    bool isStimulated() const { return state >= 0; }
    bool isOff() const { return state == 0; }
    bool isOn() const { return state == 1; }

    void resetToUnstimulated();

    // Backward-compatibility methods
    bool getState() const;
    void setState(bool s);

    bool getValue() const;
    void setValue(bool v);

    bool getOp() const;
    void setOp(bool op);

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    Point *inputPoint = nullptr;
    int state = -1; // -1: Unstimulated/Undefined (displays '?'), 0: Off, 1: On
};

#endif
