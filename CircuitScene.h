#ifndef CIRCUITSCENE_H
#define CIRCUITSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

class SelectionBox;

class CircuitScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit CircuitScene(QObject *parent = nullptr);
    ~CircuitScene();

    void setPlacementMode(bool active);
    bool isPlacementMode() const;
    void deleteItem(QGraphicsItem *item);

signals:
    void sceneClicked(const QPointF &pos);
    void placementCancelled();

private slots:
    void onSelectionChanged();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    void clearSelectionBox();

    bool m_placementMode = false;
    SelectionBox *m_activeSelectionBox = nullptr;
};

#endif // CIRCUITSCENE_H
