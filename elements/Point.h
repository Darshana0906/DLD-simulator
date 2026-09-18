#ifndef POINT_H
#define POINT_H

#include <QGraphicsItem>

class Point : public QGraphicsItem {
public:
    Point(QGraphicsItem *parent = nullptr, bool movable = true);

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
};

#endif