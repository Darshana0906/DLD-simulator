#ifndef GATE_H
#define GATE_H

#include <QGraphicsItem>
#include <QPainterPath>
#include <QGraphicsSceneMouseEvent>
#include <QList>
#include "elements/Point.h"

class Gate : public QGraphicsItem {
public:
    explicit Gate(QGraphicsItem *parent = nullptr);

    const QList<Point*>& getInputPins() const { return m_inputPins; }
    Point* getOutputPin() const { return m_outputPin; }

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    virtual QPainterPath getPath() const = 0;

    void addInputPin(qreal x, qreal y);
    void setOutputPin(qreal x, qreal y);

private:
    QList<Point*> m_inputPins;
    Point* m_outputPin = nullptr;
};

#endif // GATE_H