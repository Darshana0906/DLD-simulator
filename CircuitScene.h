#ifndef CIRCUITSCENE_H
#define CIRCUITSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

class CircuitScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit CircuitScene(QObject *parent = nullptr);

    void setPlacementMode(bool active);
    bool isPlacementMode() const;

signals:
    void sceneClicked(const QPointF &pos);
    void placementCancelled();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    bool m_placementMode = false;
};

#endif // CIRCUITSCENE_H
