#ifndef INPUT_H
#define INPUT_H

#include <QGraphicsItem>

class Point;
class Input : public QGraphicsItem {
public:
    Input(bool value = false, QGraphicsItem *parent = nullptr);
    bool getValue() const;
    void setValue(bool value);
    Point* getOutputPoint() const;

protected:
    bool value;
    Point *outputPoint;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
};

#endif