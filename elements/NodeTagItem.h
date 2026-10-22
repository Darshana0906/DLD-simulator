#ifndef NODETAGITEM_H
#define NODETAGITEM_H

#include <QGraphicsItem>
#include <QString>
#include <QFont>
#include <QRectF>

class NodeTagItem : public QGraphicsItem {
public:
    NodeTagItem(const QString &text, const QPointF &anchorPos, QGraphicsItem *parent = nullptr);

    enum { Type = UserType + 101 };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    void setTagText(const QString &text);
    QString tagText() const { return m_text; }

    void updateVisibilityFromScene();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    void calculateGeometry();

    QString m_text;
    QRectF m_boxRect;
    QRectF m_boundingRect;
    QFont m_font;
};

#endif // NODETAGITEM_H
