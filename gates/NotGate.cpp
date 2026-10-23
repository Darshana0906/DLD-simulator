#include "NotGate.h"
#include <QPainter>
#include <QMessageBox>
#include <QApplication>

NotGate::NotGate(QGraphicsItem *parent) : Gate(parent) {
    addInputPin(-29, 26);
    setOutputPin(100, 26);
}

QPainterPath NotGate::getPath() const {
    QPainterPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 60);
    path.lineTo(80, 30);
    path.closeSubpath();
    path.addEllipse(80, 25, 10, 10);
    return path;
}

void NotGate::setInputCount(int count) {
    Q_UNUSED(count);
    // NOT gate always has exactly 1 input at y = 26 (center y = 31)
    if (getInputPins().isEmpty()) {
        addInputPin(-29, 26);
    } else {
        getInputPins()[0]->setPos(-29, 26);
    }
    update();
}

void NotGate::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(Qt::black);
    painter->setBrush(Qt::white);
    painter->drawPath(getPath());
    qreal y = getInputPins().isEmpty() ? 31.0 : (getInputPins()[0]->y() + 5.0);
    painter->drawLine(-20, y, 0, y);
    painter->drawLine(90, 30, 100, 30);
}

void NotGate::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    QMessageBox msgBox(QApplication::activeWindow());
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle("Invalid Operation");
    msgBox.setText("A NOT Gate (inverter) can have only 1 input!");
    
    // Set custom size and font using CSS:
    msgBox.setStyleSheet(
        "#qt_msgbox_label { font-size: 14px; padding: 10px; }"
        "QPushButton { font-size: 13px; min-width: 75px; min-height: 26px; margin: 8px; }"
    );

    
    msgBox.exec();
    event->accept();
}