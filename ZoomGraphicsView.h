#ifndef ZOOMGRAPHICSVIEW_H
#define ZOOMGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QEnterEvent>

class CircuitScene;

class ZoomGraphicsView : public QGraphicsView
{
public:
    explicit ZoomGraphicsView(QWidget *parent = nullptr);

    // Call this after setting the scene so the view can enforce cursor
    void setCircuitScene(CircuitScene *scene);

    // Apply the correct cursor right now based on placement mode state.
    // Call this whenever placement mode changes.
    void applyCursor();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;

private:
    CircuitScene *m_circuitScene = nullptr;
};

#endif