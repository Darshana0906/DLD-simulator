#ifndef SWITCH_H
#define SWITCH_H

#include "Input.h"
#include <QGraphicsSceneMouseEvent>
#include <functional>

class Switch : public Input {
public:
    using ToggleCallback = std::function<void(Switch*, bool)>;

    explicit Switch(QGraphicsItem *parent = nullptr);

    void setValue(bool value) override;
    void setOnToggleCallback(ToggleCallback callback);
    ToggleCallback getOnToggleCallback() const;

    LogicState getLogicState() const override {
        return value ? LogicState::HIGH : LogicState::LOW;
    }

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    ToggleCallback m_onToggle = nullptr;
};

#endif // SWITCH_H