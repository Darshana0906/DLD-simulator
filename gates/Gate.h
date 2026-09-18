#ifndef GATE_H
#define GATE_H
#include <QGraphicsItem>
#include <QPainterPath>

class Gate : public QGraphicsItem {
public:
    Gate(QGraphicsItem *parent = nullptr);

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter,const QStyleOptionGraphicsItem *option,
 QWidget *widget = nullptr) override;

    virtual QPainterPath getPath() const = 0;
};
#endif