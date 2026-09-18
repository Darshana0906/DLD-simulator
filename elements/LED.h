#ifndef LED_H
#define LED_H

#include <QGraphicsItem>

class Point;
class LED : public QGraphicsItem {
public:
    LED(QGraphicsItem *parent = nullptr);
    Point* getInputPoint() const;

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    Point *inputPoint;
};

#endif