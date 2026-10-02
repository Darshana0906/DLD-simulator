#ifndef LED_H
#define LED_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>

class Point;
class QPainter;

class LED : public QGraphicsItem {
public:
    explicit LED(QGraphicsItem *parent = nullptr);
    Point* getInputPoint() const;

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
    Point *inputPoint;
    bool state = false;
};

#endif
